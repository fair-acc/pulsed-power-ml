#ifndef TIME_DOMAIN_WORKER_H
#define TIME_DOMAIN_WORKER_H

#include <disruptor/RingBuffer.hpp>
#include <majordomo/Worker.hpp>

#include <gnuradio/pulsed_power/opencmw_time_sink.h>

#include <chrono>
#include <unordered_map>

using opencmw::Annotated;
using opencmw::NoUnit;

struct TimeDomainContext {
    std::string             channelNameFilter;
    int32_t                 acquisitionModeFilter = 0; // STREAMING
    std::string             triggerNameFilter;
    int32_t                 maxClientUpdateFrequencyFilter = 25;
    opencmw::MIME::MimeType contentType                    = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(TimeDomainContext, channelNameFilter, acquisitionModeFilter, triggerNameFilter, maxClientUpdateFrequencyFilter, contentType)

struct Acquisition {
    std::string                   refTriggerName  = { "NO_REF_TRIGGER" };
    int64_t                       refTriggerStamp = 0;
    std::vector<float>            channelTimeSinceRefTrigger;
    float                         channelUserDelay   = 0.0f;
    float                         channelActualDelay = 0.0f;
    std::vector<std::string>      channelNames;
    opencmw::MultiArray<float, 2> channelValues;
    opencmw::MultiArray<float, 2> channelErrors;
    std::vector<std::string>      channelUnits;
    std::vector<int64_t>          status;
    std::vector<float>            channelRangeMin;
    std::vector<float>            channelRangeMax;
    std::vector<float>            temperature;
};

ENABLE_REFLECTION_FOR(Acquisition, refTriggerName, refTriggerStamp, channelTimeSinceRefTrigger, channelUserDelay, channelActualDelay, channelNames, channelValues, channelErrors, channelUnits, status, channelRangeMin, channelRangeMax, temperature)

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class TimeDomainWorker
    : public Worker<serviceName, TimeDomainContext, Empty, Acquisition, Meta...> {
private:
    static const size_t RING_BUFFER_SIZE = 4096;
    const std::string   _deviceName;
    std::atomic<bool>   _shutdownRequested;
    std::jthread        _pollingThread;
    Acquisition         _reply;
    struct RingBufferData {
        std::vector<float> chunk;
        int64_t            timestamp;
    };
    using ringbuffer_t  = std::shared_ptr<RingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;
    using eventpoller_t = std::shared_ptr<EventPoller<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;
    struct SignalData {
        std::string   signalName;
        std::string   signalUnit;
        float         sampleRate;
        ringbuffer_t  ringBuffer;
        eventpoller_t eventPoller;
    };

    std::unordered_map<std::string, SignalData> _signalsMap; // <completeSignalName, signalData>

public:
    using super_t = Worker<serviceName, TimeDomainContext, Empty, Acquisition, Meta...>;

    template<typename BrokerType>
    explicit TimeDomainWorker(const BrokerType &broker)
        : super_t(broker, {}) {
        // polling thread
        _pollingThread = std::jthread([this] {
            std::chrono::duration<double, std::milli> pollingDuration;
            while (!_shutdownRequested) {
                std::chrono::time_point time_start = std::chrono::system_clock::now();
                for (auto subTopic : super_t::activeSubscriptions()) { // loop over active subscriptions

                    if (subTopic.path() != "/Acquisition") {
                        break;
                    }

                    const auto                         queryMap = subTopic.queryParamMap();
                    const TimeDomainContext            filterIn = opencmw::query::deserialise<TimeDomainContext>(queryMap);

                    std::set<std::string, std::less<>> requestedSignals;
                    if (!checkRequestedSignals(filterIn, requestedSignals)) {
                        break;
                    }

                    uint64_t maxChunksToPoll = chunksToPoll(requestedSignals);

                    if (maxChunksToPoll == 0) {
                        break;
                    }

                    pollMultipleSignals(requestedSignals, maxChunksToPoll, _reply);
                    TimeDomainContext filterOut = filterIn;
                    filterOut.contentType       = opencmw::MIME::JSON;
                    super_t::notify("/Acquisition", filterOut, _reply);
                }
                pollingDuration   = std::chrono::system_clock::now() - time_start;

                auto willSleepFor = std::chrono::milliseconds(40) - pollingDuration;
                std::this_thread::sleep_for(willSleepFor);
            }
        });

        // map signal names and ringbuffers, register callback
        std::scoped_lock lock(gr::pulsed_power::globalTimeSinksRegistryMutex);
        fmt::print("GR: number of time-domain sinks found: {}\n", gr::pulsed_power::globalTimeSinksRegistry.size());
        for (auto sink : gr::pulsed_power::globalTimeSinksRegistry) {
            const auto signal_name = sink->get_signal_name();
            const auto signal_unit = sink->get_signal_unit();
            const auto sample_rate = sink->get_sample_rate();

            // init RingBuffer, register poller and poller sequence
            auto ringbuffer = newRingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, ProducerType::Single>();
            auto poller     = ringbuffer->newPoller();
            ringbuffer->addGatingSequences({ poller->sequence() });

            const auto completeSignalName   = fmt::format("{}@{}Hz", signal_name, sample_rate);
            _signalsMap[completeSignalName] = SignalData({ signal_name, signal_unit, sample_rate, ringbuffer, poller });
            fmt::print("GR: OpenCMW Time Sink '{}' added\n", completeSignalName);

            // register callback
            sink->set_callback(std::bind(&TimeDomainWorker::callbackCopySinkData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        }

        super_t::setCallback([this](RequestContext &rawCtx, const TimeDomainContext     &requestContext, const Empty &,
                                     TimeDomainContext /* &replyContext */, Acquisition &out) {
            if (rawCtx.request.command() == Command::Get) {
                handleGetRequest(requestContext, out);
            }
        });
    }

    ~TimeDomainWorker() {
        _shutdownRequested = true;
        _pollingThread.join();
    }

    void callbackCopySinkData(const float *data, int data_size, const std::string &signal_name, float sample_rate, int64_t timestamp_ns) {
        if (sample_rate > 0) {
            // realign timestamp
            timestamp_ns -= static_cast<int64_t>(1 / sample_rate * 1e9F) * data_size;
        }

        // write into RingBuffer
        const auto completeSignalName = fmt::format("{}@{}Hz", signal_name, sample_rate);
        if (_signalsMap.contains(completeSignalName)) {
            const SignalData &signalData = _signalsMap.at(completeSignalName);

            // publish data
            bool result = signalData.ringBuffer->tryPublishEvent([&data, data_size, timestamp_ns](RingBufferData &&bufferData, std::int64_t /*sequence*/) noexcept {
                bufferData.timestamp = timestamp_ns;
                bufferData.chunk.assign(data, data + data_size);
            });

            if (!result) {
                // fmt::print("error writing into RingBuffer, signal_name: {}\n", signal_name);
            }
        }
    }

private:
    bool handleGetRequest(const TimeDomainContext &requestContext, Acquisition &out) {
        std::set<std::string, std::less<>> requestedSignals;
        if (!checkRequestedSignals(requestContext, requestedSignals)) {
            return false; // TODO throw exception
        }

        uint64_t maxChunksToPoll = chunksToPoll(requestedSignals);

        if (maxChunksToPoll == 0) {
            return false;
        }

        pollMultipleSignals(requestedSignals, maxChunksToPoll, out);
        return true;
    }

    // find how many chunks should be parallely polled
    uint64_t chunksToPoll(std::set<std::string, std::less<>> &requestedSignals) {
        std::vector<uint64_t> chunksAvailable;
        for (const auto &requestedSignal : requestedSignals) {
            auto     signalData = _signalsMap.at(requestedSignal);
            uint64_t diff       = signalData.ringBuffer->cursor() - signalData.eventPoller->sequence()->value();
            chunksAvailable.push_back(diff);
        }
        assert(!chunksAvailable.empty());
        auto maxChunksToPollIterator = std::min_element(chunksAvailable.begin(), chunksAvailable.end());
        if (maxChunksToPollIterator == chunksAvailable.end()) {
            return 0;
        }

        return *maxChunksToPollIterator;
    }

    void pollMultipleSignals(const std::set<std::string, std::less<>> &requestedSignals, uint64_t chunksToPoll, Acquisition &out) {
        std::vector<float> stridedValues;
        out.refTriggerStamp = 0;
        out.channelNames.clear();
        float sampleRate = 0;
        for (const auto &requestedSignal : requestedSignals) {
            auto signalData = _signalsMap.at(requestedSignal);
            assert(chunksToPoll > 0);
            sampleRate = signalData.sampleRate;

            out.channelNames.push_back(requestedSignal);

            bool      firstChunk = true;
            PollState result     = PollState::Idle;
            for (size_t i = 0; i < chunksToPoll; i++) {
                result = signalData.eventPoller->poll([&](RingBufferData &event, std::int64_t /*sequence*/, bool /*nomoreEvts*/) noexcept {
                    if (firstChunk) {
                        out.refTriggerStamp = event.timestamp;
                        stridedValues.reserve(requestedSignals.size() * chunksToPoll * event.chunk.size());
                        firstChunk = false;
                    }

                    stridedValues.insert(stridedValues.end(), event.chunk.begin(), event.chunk.end());

                    return false;
                });
            }
            assert(result == PollState::Processing);
        }

        //  generate multiarray values from strided array
        size_t channelValuesSize = stridedValues.size() / requestedSignals.size();
        out.channelValues        = opencmw::MultiArray<float, 2>(std::move(stridedValues), { static_cast<uint32_t>(requestedSignals.size()), static_cast<uint32_t>(channelValuesSize) });
        //  generate relative timestamps
        out.channelTimeSinceRefTrigger.clear();
        out.channelTimeSinceRefTrigger.reserve(channelValuesSize);
        for (size_t i = 0; i < channelValuesSize; ++i) {
            float relativeTimestamp = static_cast<float>(i) * (1 / sampleRate);
            out.channelTimeSinceRefTrigger.push_back(relativeTimestamp);
        }
    }

    bool checkRequestedSignals(const TimeDomainContext &filterIn, std::set<std::string, std::less<>> &requestedSignals) {
        auto signals = std::string_view(filterIn.channelNameFilter) | std::ranges::views::split(',');
        for (const auto &signal : signals) {
            requestedSignals.emplace(std::string_view(signal.begin(), signal.end()));
        }
        if (requestedSignals.empty()) {
            respondWithEmptyResponse(filterIn, "no signals requested, sending empty response\n");
            return false;
        }

        // check if signals exist
        std::vector<std::string> unknownSignals;
        for (const auto &requestedSignal : requestedSignals) {
            if (!_signalsMap.contains(requestedSignal)) {
                unknownSignals.push_back(requestedSignal);
            }
        }
        if (!unknownSignals.empty()) {
            respondWithEmptyResponse(filterIn, fmt::format("requested unknown signals: {}\n", unknownSignals));
            return false;
        }

        return true;
    }

    void respondWithEmptyResponse(const TimeDomainContext &filter, const std::string_view errorText) {
        fmt::print("{}\n", errorText);
        super_t::notify("/Acquisition", filter, Acquisition());
    }
};

#endif /* TIME_DOMAIN_WORKER_H */
