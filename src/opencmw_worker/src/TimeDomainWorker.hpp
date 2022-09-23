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
    int32_t                 maxClientUpdateFrequencyFilter;
    opencmw::MIME::MimeType contentType = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(TimeDomainContext, channelNameFilter, acquisitionModeFilter, triggerNameFilter, maxClientUpdateFrequencyFilter, contentType)

struct Acquisition {
    std::string                   refTriggerName = { "NO_REF_TRIGGER" };
    int64_t                       refTriggerStamp;
    std::vector<float>            channelTimeSinceRefTrigger;
    float                         channelUserDelay;
    float                         channelActualDelay;
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

struct RingBufferData {
    std::vector<float> chunk;
    int64_t            timestamp;
};

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
    explicit TimeDomainWorker(const BrokerType &broker, const std::string &deviceName = "")
        : super_t(broker, {}), _deviceName(deviceName), _reply{} {
        // polling thread
        _pollingThread = std::jthread([this] {
            std::chrono::duration<double, std::milli> pollingDuration;
            while (!_shutdownRequested) {
                std::chrono::time_point time_start = std::chrono::system_clock::now();

                for (auto subTopic : super_t::activeSubscriptions()) { // loop over active subscriptions

                    const auto                         queryMap = subTopic.queryParamMap();
                    const TimeDomainContext            filterIn = opencmw::query::deserialise<TimeDomainContext>(queryMap);

                    std::set<std::string, std::less<>> requestedSignals;
                    if (!checkRequestedSignals(filterIn, requestedSignals)) {
                        break;
                    }

                    // find how many chunks should be parallely polled
                    std::vector<uint64_t> chunksAvailable;
                    for (const auto &requestedSignal : requestedSignals) {
                        auto     signalData = _signalsMap.at(requestedSignal);
                        uint64_t diff       = signalData.ringBuffer->cursor() - signalData.eventPoller->sequence()->value();
                        chunksAvailable.push_back(diff);
                    }

                    uint64_t maxChunksToPoll = *std::min_element(chunksAvailable.begin(), chunksAvailable.end());

                    if (maxChunksToPoll == 0) {
                        // no new items for requested signals
                        break;
                    }

                    // poll data
                    std::vector<float> stridedValues;
                    _reply.refTriggerStamp = 0;
                    _reply.channelNames.clear();
                    float sampleRate;
                    for (const auto &requestedSignal : requestedSignals) {
                        auto signalData = _signalsMap.at(requestedSignal);
                        assert(maxChunksToPoll > 0);
                        sampleRate = signalData.sampleRate;

                        _reply.channelNames.push_back(requestedSignal);

                        bool      firstChunk = true;
                        PollState result;
                        for (size_t i = 0; i < maxChunksToPoll; i++) {
                            result = signalData.eventPoller->poll([&](RingBufferData &event, std::int64_t /*sequence*/, bool /*nomoreEvts*/) noexcept {
                                if (firstChunk) {
                                    _reply.refTriggerStamp = event.timestamp;
                                    stridedValues.reserve(requestedSignals.size() * maxChunksToPoll * event.chunk.size());
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
                    _reply.channelValues     = opencmw::MultiArray<float, 2>(std::move(stridedValues), { requestedSignals.size(), channelValuesSize });
                    //  generate relative timestamps
                    _reply.channelTimeSinceRefTrigger.clear();
                    _reply.channelTimeSinceRefTrigger.reserve(channelValuesSize);
                    for (size_t i = 0; i < channelValuesSize; ++i) {
                        float relativeTimestamp = static_cast<float>(i) * (1 / sampleRate);
                        _reply.channelTimeSinceRefTrigger.push_back(relativeTimestamp);
                    }
                    TimeDomainContext filterOut = filterIn;
                    filterOut.contentType       = opencmw::MIME::JSON;
                    // notify == zmq publish
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

            const auto completeSignalName   = getCompleteSignalName(signal_name, sample_rate);
            _signalsMap[completeSignalName] = SignalData({ signal_name, signal_unit, sample_rate, ringbuffer, poller });
            fmt::print("GR: OpenCMW Time Sink '{}' added\n", completeSignalName);

            // register callback
            sink->set_callback(std::bind(&TimeDomainWorker::callbackCopySinkData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        }

        super_t::setCallback([this](RequestContext & /*rawCtx*/, const TimeDomainContext & /*requestContext*/, const Empty &,
                                     TimeDomainContext & /*replyContext*/, Acquisition &out) {
            out = _reply;
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
        if (_signalsMap.find(getCompleteSignalName(signal_name, sample_rate)) != _signalsMap.end()) {
            const SignalData &signalData = _signalsMap.at(getCompleteSignalName(signal_name, sample_rate));

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
    const std::string getCompleteSignalName(const std::string &signalName, float sampleRate) const {
        return fmt::format("{}{}{}@{}Hz", _deviceName, _deviceName.empty() ? "" : ":", signalName, sampleRate);
        // return fmt::format("{}", signalName);
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
            if (_signalsMap.find(requestedSignal) == _signalsMap.end()) {
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
