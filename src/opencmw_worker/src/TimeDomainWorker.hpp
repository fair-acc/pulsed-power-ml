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
    int64_t                 lastRefTrigger                 = 0;
    opencmw::MIME::MimeType contentType                    = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(TimeDomainContext, channelNameFilter, acquisitionModeFilter, triggerNameFilter, maxClientUpdateFrequencyFilter, lastRefTrigger, contentType)

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
    int64_t                       lastTimeStamp = 0;
};

ENABLE_REFLECTION_FOR(Acquisition, refTriggerName, refTriggerStamp, channelTimeSinceRefTrigger, channelUserDelay, channelActualDelay, channelNames, channelValues, channelErrors, channelUnits, status, channelRangeMin, channelRangeMax, temperature)

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class TimeDomainWorker
    : public Worker<serviceName, TimeDomainContext, Empty, Acquisition, Meta...> {
private:
    static const size_t RING_BUFFER_SIZE = 256;
    std::atomic<bool>   _shutdownRequested;
    std::jthread        _pollingThread;

    class GRSink {
        struct RingBufferData {
            std::vector<std::vector<float>> chunk;
            int64_t                         timestamp = 0;
        };
        using ringbuffer_t = std::shared_ptr<RingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;
        using sequence_t   = std::shared_ptr<Sequence>;

        std::vector<std::string> _channelNames;      // { signalName1, signalName2, ... }
        std::vector<std::string> _channelUnits;      // { signalUnit1, signalUnit2, ... }
        std::string              _channelNameFilter; // signalName1@sampleRate,signalName2@sampleRate...
        float                    _sampleRate = 0;
        ringbuffer_t             _ringBuffer;
        sequence_t               _tail;

    public:
        GRSink() = delete;
        GRSink(gr::pulsed_power::opencmw_time_sink *sink)
            : _channelNames(sink->get_signal_names()), _sampleRate(sink->get_sample_rate()), _ringBuffer(newRingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, ProducerType::Single>()), _tail(std::make_shared<Sequence>()) {
            _ringBuffer->addGatingSequences({ _tail });

            for (size_t i = 0; i < _channelNames.size(); i++) {
                _channelNameFilter.append(fmt::format("{}@{}Hz", _channelNames[i], _sampleRate));
                _channelUnits = sink->get_signal_units();
                if (i != (_channelNames.size() - 1)) {
                    _channelNameFilter.append(",");
                }
            }
        };

        std::string getChannelNameFilter() const {
            return _channelNameFilter;
        };

        ringbuffer_t getRingBuffer() const {
            return _ringBuffer;
        };

        void fetchData(const int64_t lastRefTrigger, Acquisition &out) {
            std::vector<float> stridedValues;
            int64_t            tail       = _tail->value();
            int64_t            head       = _ringBuffer->cursor();

            bool               firstChunk = true;
            for (size_t i = 0; i < _channelNames.size(); i++) {
                for (int64_t sequence = tail; sequence <= head; sequence++) {
                    const RingBufferData &bufData = (*_ringBuffer)[sequence];

                    if (bufData.timestamp > lastRefTrigger) {
                        if (firstChunk) {
                            for (const auto &channelName : _channelNames) {
                                out.channelNames.push_back(fmt::format("{}@{}Hz", channelName, _sampleRate));
                            }
                            out.channelUnits    = _channelUnits;
                            out.refTriggerStamp = bufData.timestamp;
                            firstChunk          = false;
                        }

                        stridedValues.insert(stridedValues.end(), bufData.chunk[i].begin(), bufData.chunk[i].end());
                    }
                }
            }

            if (!stridedValues.empty()) {
                //  generate multiarray values from strided array
                size_t channelValuesSize = stridedValues.size() / _channelNames.size();
                out.channelValues        = opencmw::MultiArray<float, 2>(std::move(stridedValues), { static_cast<uint32_t>(_channelNames.size()), static_cast<uint32_t>(channelValuesSize) });
                //  generate relative timestamps
                out.channelTimeSinceRefTrigger.reserve(channelValuesSize);
                for (size_t i = 0; i < channelValuesSize; ++i) {
                    float relativeTimestamp = static_cast<float>(i) / _sampleRate;
                    out.channelTimeSinceRefTrigger.push_back(relativeTimestamp);
                }
            } else {
                // throw std::invalid_argument(fmt::format("No new data available for signals: '{}'", _channelNames));
            }
        };

        void copySinkData(std::vector<const void *> &input_items, int &noutput_items, const std::vector<std::string> &signal_names, float /* sample_rate */, int64_t timestamp_ns) {
            if (signal_names == _channelNames) {
                bool result = _ringBuffer->tryPublishEvent([&input_items, noutput_items, timestamp_ns](RingBufferData &&bufferData, std::int64_t /*sequence*/) noexcept {
                    bufferData.timestamp = timestamp_ns;
                    bufferData.chunk.clear();
                    for (size_t i = 0; i < input_items.size(); i++) {
                        const float *in = static_cast<const float *>(input_items[i]);
                        bufferData.chunk.emplace_back(std::vector<float>(in, in + noutput_items));
                    }
                });

                if (result) {
                    auto       headValue       = _ringBuffer->cursor();
                    auto       tailValue       = _tail->value();

                    const auto tailOffsetValue = static_cast<int64_t>(RING_BUFFER_SIZE) * 50 / 100; // 50 %

                    if (headValue > (tailValue + tailOffsetValue)) {
                        _tail->setValue(headValue - tailOffsetValue);
                    }

                } else {
                    // error writing into RingBuffer
                    fmt::print("timeDomainWorker: writing into RingBuffer failed, signal_name: {}\n", signal_names[0]);
                }
            }
        }
    };

    std::unordered_map<std::string, GRSink> _sinksMap; // <subscriptionName, GRSink>

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

                    const auto              queryMap = subTopic.queryParamMap();
                    const TimeDomainContext filterIn = opencmw::query::deserialise<TimeDomainContext>(queryMap);

                    try {
                        Acquisition reply;
                        handleGetRequest(filterIn, reply);

                        TimeDomainContext filterOut = filterIn;
                        filterOut.contentType       = opencmw::MIME::JSON;
                        super_t::notify("/Acquisition", filterOut, reply);
                    } catch (const std::exception &ex) {
                        fmt::print("caught exception '{}'\n", ex.what());
                    }
                }

                pollingDuration   = std::chrono::system_clock::now() - time_start;

                auto willSleepFor = std::chrono::milliseconds(40) - pollingDuration;
                if (willSleepFor > std::chrono::milliseconds(0)) {
                    std::this_thread::sleep_for(willSleepFor);
                }
            }
        });

        // map signal names and ringbuffers, register callback
        std::scoped_lock lock(gr::pulsed_power::globalTimeSinksRegistryMutex);
        fmt::print("GR: OpenCMW: time-domain sinks found: {}\n", gr::pulsed_power::globalTimeSinksRegistry.size());

        for (const auto timeSink : gr::pulsed_power::globalTimeSinksRegistry) {
            GRSink grSink(timeSink);
            auto   completeSubscriptionName = grSink.getChannelNameFilter();
            _sinksMap.insert({ completeSubscriptionName, grSink });
            fmt::print("GR: OpenCMW Time Sink subscription '{}' added\n", completeSubscriptionName);

            // register callback
            timeSink->set_callback(std::bind(&GRSink::copySinkData, grSink, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
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

private:
    bool handleGetRequest(const TimeDomainContext &requestContext, Acquisition &out) {
        if (_sinksMap.contains(requestContext.channelNameFilter)) {
            auto &sink = _sinksMap.at(requestContext.channelNameFilter);
            sink.fetchData(requestContext.lastRefTrigger, out);
        } else {
            throw std::invalid_argument(fmt::format("Requested subscription for '{}' not found", requestContext.channelNameFilter));
        }
        return true;
    }
};

#endif /* TIME_DOMAIN_WORKER_H */
