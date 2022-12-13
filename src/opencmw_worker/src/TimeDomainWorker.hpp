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

ENABLE_REFLECTION_FOR(Acquisition, refTriggerName, refTriggerStamp, channelTimeSinceRefTrigger, channelUserDelay, channelActualDelay, channelNames, channelValues, channelErrors, channelUnits, status, channelRangeMin, channelRangeMax, temperature, lastTimeStamp)

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class TimeDomainWorker
    : public Worker<serviceName, TimeDomainContext, Empty, Acquisition, Meta...> {
private:
    static const size_t RING_BUFFER_SIZE = 128;
    std::atomic<bool>   _shutdownRequested;
    std::jthread        _pollingThread;
    std::jthread        _signalDataStatus;

    struct RingBufferData {
        size_t                          nsignals = 0;
        std::vector<std::vector<float>> chunk;
        int64_t                         timestamp = 0;
    };

    using ringbuffer_t  = std::shared_ptr<RingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;
    using eventpoller_t = std::shared_ptr<EventPoller<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;
    using sequence_t    = std::shared_ptr<Sequence>;
    class GRSignal {
        std::string  _signalName;
        std::string  _signalUnit;
        ringbuffer_t _ringBuffer;
        sequence_t   _tail;

    public:
        GRSignal() = delete;
        GRSignal(const std::string signalName, const std::string signalUnit)
            : _signalName(signalName), _signalUnit(signalUnit), _ringBuffer(newRingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, ProducerType::Single>()), _tail(std::make_shared<Sequence>()) {
            _ringBuffer->addGatingSequences({ _tail });
        };

        sequence_t getTailSequence() const {
            return _tail;
        };

        ringbuffer_t getRingBuffer() const {
            return _ringBuffer;
        };

        std::string getSignalName() const {
            return _signalName;
        };
    };

    class GRSink {
        std::vector<std::string> _subscriptionNames; // { signalName1, signalName2, ... }
        std::string              _channelNameFilter; // signalName1@sampleRate,signalName2@sampleRate...
        float                    _sampleRate = 0;
        ringbuffer_t             _ringBuffer;
        eventpoller_t            _eventPoller;
        sequence_t               _tail;

        // std::unordered_map<std::string, GRSignal> _signalsMap;

    public:
        GRSink() = delete;
        GRSink(gr::pulsed_power::opencmw_time_sink *sink)
            : _subscriptionNames(sink->get_signal_names()), _sampleRate(sink->get_sample_rate()), _ringBuffer(newRingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, ProducerType::Single>()), _eventPoller(_ringBuffer->newPoller()), _tail(std::make_shared<Sequence>()) {
            _ringBuffer->addGatingSequences({ /* _eventPoller->sequence(), */ _tail });

            for (size_t i = 0; i < _subscriptionNames.size(); i++) {
                _channelNameFilter.append(fmt::format("{}@{}Hz", _subscriptionNames[i], _sampleRate));
                if (i != (_subscriptionNames.size() - 1)) {
                    _channelNameFilter.append(",");
                }
                // _signalsMap.insert({ _subscriptionNames[i], GRSignal(_subscriptionNames[i], sink->get_signal_units()[i]) });
            }
        };

        std::string getChannelNameFilter() const {
            return _channelNameFilter;
        };

        ringbuffer_t getRingBuffer() const {
            return _ringBuffer;
        };

        // std::unordered_map<std::string, GRSignal> getSignalsMap() const {
        //     return _signalsMap;
        // };

        void fetchData(const int64_t lastRefTrigger, Acquisition &out) {
            std::vector<float> stridedValues;
            int64_t            begin      = _tail->value();
            int64_t            end        = _ringBuffer->cursor();
            bool               firstChunk = true;
            for (size_t i = 0; i < _subscriptionNames.size(); i++) {
                fmt::print("fetch signal {} from seq {} to {}\n", _subscriptionNames[i], begin, end);
                out.channelNames.push_back(fmt::format("{}@{}Hz", _subscriptionNames[i], _sampleRate));

                // eventpoller_t poller = _ringBuffer->newPoller()
                // PollState result = PollState::Idle;
                // result =

                for (int64_t seq = begin; seq < end; seq++) {
                    const RingBufferData &bufData = (*_ringBuffer)[seq];
                    if (bufData.timestamp > lastRefTrigger) {
                        assert(bufData.nsignals == _subscriptionNames.size());
                        if (firstChunk) {
                            // fmt::print("first fetch from seq {}\n", seq);
                            out.refTriggerStamp = bufData.timestamp;
                            if (bufData.timestamp == 0) {
                                // fmt::print("signal {}, bufData timestamp == 0\n", _subscriptionNames[i]);
                            }

                            firstChunk = false;
                        }
                        stridedValues.insert(stridedValues.end(), bufData.chunk[i].begin(), bufData.chunk[i].end());
                    }
                }
                if (stridedValues.empty()) {
                    // throw std::invalid_argument(fmt::format("No signal data"));
                }
            }

            //     out.channelNames.push_back(signalName);
            //     bool    firstChunk = true;
            //     int64_t begin      = _tail->value();
            //     int64_t end        = _ringBuffer->cursor();
            //     fmt::print("fetch signal {} from seq {} to {}\n", signalName, begin, end);
            //     for (int64_t seq = begin; seq <= end; seq++) {
            //         const RingBufferData &data = (*ringBuffer)[seq];
            //         if (data.timestamp > lastRefTrigger) {
            //             if (firstChunk) {
            //                 fmt::print("first fetch from seq {}\n", seq);
            //                 out.refTriggerStamp = data.timestamp;
            //                 firstChunk          = false;
            //             }
            //             stridedValues.insert(stridedValues.end(), data.chunk.begin(), data.chunk.end());
            //         }
            //     }

            //  generate multiarray values from strided array
            size_t channelValuesSize = stridedValues.size() / _subscriptionNames.size();
            out.channelValues        = opencmw::MultiArray<float, 2>(std::move(stridedValues), { static_cast<uint32_t>(_subscriptionNames.size()), static_cast<uint32_t>(channelValuesSize) });
            //  generate relative timestamps
            // out.channelTimeSinceRefTrigger.clear();
            out.channelTimeSinceRefTrigger.reserve(channelValuesSize);
            for (size_t i = 0; i < channelValuesSize; ++i) {
                float relativeTimestamp = static_cast<float>(i) / _sampleRate;
                out.channelTimeSinceRefTrigger.push_back(relativeTimestamp);
            }
        };

        void copySinkData(std::vector<const void *> &input_items, int &noutput_items, const std::vector<std::string> &signal_names, float sample_rate, int64_t timestamp_ns) {
            if (signal_names == _subscriptionNames) {
                bool result = _ringBuffer->tryPublishEvent([&input_items, noutput_items, timestamp_ns](RingBufferData &&bufferData, std::int64_t /*sequence*/) noexcept {
                    bufferData.nsignals  = input_items.size();
                    bufferData.timestamp = timestamp_ns;
                    bufferData.chunk.clear();
                    for (size_t i = 0; i < bufferData.nsignals; i++) {
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
                    fmt::print("error writing into RingBuffer, signal_names: {}\n", signal_names);
                    noutput_items = 0;
                }
            }
        }
    };

    std::unordered_map<std::string, GRSink> _sinksMap; // <subscriptionName, signalData>

public:
    using super_t = Worker<serviceName, TimeDomainContext, Empty, Acquisition, Meta...>;

    template<typename BrokerType>
    explicit TimeDomainWorker(const BrokerType &broker)
        : super_t(broker, {}) {
        // polling thread
        _pollingThread    = std::jthread([this] {
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

                    // int64_t maxChunksToPoll = chunksToPoll(requestedSignals);

                    // if (maxChunksToPoll == 0) {
                    //     break;
                    // }

                    Acquisition reply;
                    // pollMultipleSignals(requestedSignals, maxChunksToPoll, reply);
                    if (_sinksMap.contains(filterIn.channelNameFilter)) {
                        fmt::print("signals requested: {}, lastRefTrigger {}\n", filterIn.channelNameFilter, filterIn.lastRefTrigger);
                        auto &sink = _sinksMap.at(filterIn.channelNameFilter);
                        sink.fetchData(filterIn.lastRefTrigger, reply);
                    }
                    TimeDomainContext filterOut = filterIn;
                    filterOut.contentType       = opencmw::MIME::JSON;
                    super_t::notify("/Acquisition", filterOut, reply);
                }

                pollingDuration   = std::chrono::system_clock::now() - time_start;

                auto willSleepFor = std::chrono::milliseconds(40) - pollingDuration;
                std::this_thread::sleep_for(willSleepFor);
            }
           });

        _signalDataStatus = std::jthread([this] {
            while (!_shutdownRequested) {
                std::this_thread::sleep_for(1000ms);
                for (auto const &[subscription, sink] : _sinksMap) {
                    // fmt::print("signalDataStatus: subscription: {}\n", subscription);
                    const auto        ringBuffer = sink.getRingBuffer();
                    auto              cursor     = ringBuffer->cursor();
                    auto              size       = ringBuffer->bufferSize();
                    auto              used       = cursor - ringBuffer->getMinimumGatingSequence();
                    auto              firstItem  = true;
                    std::stringstream gatingSequences;
                    for (const auto &sequence : *ringBuffer->getGatingSequences()) {
                        if (firstItem) {
                            firstItem = false;
                        } else {
                            gatingSequences << ", ";
                        }
                        gatingSequences << fmt::format("{{ {} }}", sequence->value());
                    }
                    // fmt::print("subscription: {:<30}, ringbuffer Cursor: {}, Used {}/{}, {}%, GatingSequences: {}\n",
                    //         subscription,
                    //         cursor,
                    //         used,
                    //         size,
                    //         used * 100 / size,
                    //         gatingSequences.str());
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
                // fmt::print("out.refTrigger: {}\n", out.refTriggerStamp);
            }
        });
    }

    ~TimeDomainWorker() {
        _shutdownRequested = true;
        _pollingThread.join();
        _signalDataStatus.join();
    }

    // void callbackCopySinkData(const float *data, int &noutput_items, const std::string &signal_name, float sample_rate, int64_t timestamp_ns) {
    //     const auto completeSignalName = fmt::format("{}@{}Hz", signal_name, sample_rate);
    //     if (_signalsMap.contains(completeSignalName)) {
    //         const SignalData &signalData = _signalsMap.at(completeSignalName);
    //         // publish data
    //         bool result = signalData.getRingBuffer()->tryPublishEvent([&data, noutput_items, timestamp_ns](RingBufferData &&bufferData, std::int64_t /*sequence*/) noexcept {
    //             bufferData.timestamp = timestamp_ns;
    //             bufferData.chunk.assign(data, data + noutput_items);
    //         });

    //         if (result) {
    //             auto       headValue       = signalData.getRingBuffer()->cursor();
    //             auto       tailSequence    = signalData.getTailSequence();
    //             auto       tailValue       = signalData.getTailSequence()->value();

    //             const auto tailOffsetValue = static_cast<int64_t>(RING_BUFFER_SIZE) * 50 / 100; // %

    //             if (headValue > (tailValue + tailOffsetValue)) {
    //                 tailSequence->setValue(headValue - tailOffsetValue);
    //             }

    //         } else {
    //             fmt::print("error writing into RingBuffer, signal_name: {}\n", signal_name);
    //             noutput_items = 0;
    //         }
    //     }
    // }

private:
    bool handleGetRequest(const TimeDomainContext &requestContext, Acquisition &out) {
        std::set<std::string, std::less<>> requestedSignals;
        if (!checkRequestedSignals(requestContext, requestedSignals)) {
            return false; // TODO throw exception
        }

        // int64_t maxChunksToPoll = chunksToPoll(requestedSignals);

        // if (maxChunksToPoll == 0) {
        //     return false;
        // }

        // pollMultipleSignals(requestedSignals, maxChunksToPoll, out);
        if (_sinksMap.contains(requestContext.channelNameFilter)) {
            fmt::print("signals requested: {}, lastRefTrigger {}\n", requestContext.channelNameFilter, requestContext.lastRefTrigger);
            auto &sink = _sinksMap.at(requestContext.channelNameFilter);
            sink.fetchData(requestContext.lastRefTrigger, out);
        }
        return true;
    }

    // find how many chunks should be parallely polled
    int64_t chunksToPoll(std::set<std::string, std::less<>> &requestedSignals) {
        std::vector<int64_t> chunksAvailable;
        for (const auto &requestedSignal : requestedSignals) {
            // auto    signalData = _signalsMap.at(requestedSignal);
            // int64_t diff       = signalData.getRingBuffer()->cursor() - signalData.getEventPoller()->sequence()->value();
            // chunksAvailable.push_back(diff);
        }
        assert(!chunksAvailable.empty());
        auto maxChunksToPollIterator = std::min_element(chunksAvailable.begin(), chunksAvailable.end());
        if (maxChunksToPollIterator == chunksAvailable.end()) {
            return 0;
        }

        return *maxChunksToPollIterator;
    }

    void pollMultipleSignals(const std::set<std::string, std::less<>> &requestedSignals, int64_t chunksToPoll, Acquisition &out) {
        // std::vector<float> stridedValues;
        // out.refTriggerStamp = 0;
        // out.channelNames.clear();
        // float sampleRate = 0;
        // for (const auto &requestedSignal : requestedSignals) {
        //     auto signalData = _signalsMap.at(requestedSignal);
        //     assert(chunksToPoll > 0);
        //     sampleRate = signalData.getSampleRate();

        //     out.channelNames.push_back(requestedSignal);

        //     bool      firstChunk = true;
        //     PollState result     = PollState::Idle;
        //     for (int64_t i = 0; i < chunksToPoll; i++) {
        //         result = signalData.getEventPoller()->poll([&](RingBufferData &event, std::int64_t /*sequence*/, bool /*nomoreEvts*/) noexcept {
        //             if (firstChunk) {
        //                 out.refTriggerStamp = event.timestamp;
        //                 stridedValues.reserve(requestedSignals.size() * static_cast<size_t>(chunksToPoll) * event.chunk.size());
        //                 firstChunk = false;
        //             }

        //             stridedValues.insert(stridedValues.end(), event.chunk.begin(), event.chunk.end());

        //             return false;
        //         });
        //     }
        //     assert(result == PollState::Processing);
        // }

        // //  generate multiarray values from strided array
        // size_t channelValuesSize = stridedValues.size() / requestedSignals.size();
        // out.channelValues        = opencmw::MultiArray<float, 2>(std::move(stridedValues), { static_cast<uint32_t>(requestedSignals.size()), static_cast<uint32_t>(channelValuesSize) });
        // //  generate relative timestamps
        // out.channelTimeSinceRefTrigger.clear();
        // out.channelTimeSinceRefTrigger.reserve(channelValuesSize);
        // for (size_t i = 0; i < channelValuesSize; ++i) {
        //     float relativeTimestamp = static_cast<float>(i) * (1 / sampleRate);
        //     out.channelTimeSinceRefTrigger.push_back(relativeTimestamp);
        // }
    }

    bool checkRequestedSignals(const TimeDomainContext &filterIn, std::set<std::string, std::less<>> &requestedSignals) {
        // auto signals = std::string_view(filterIn.channelNameFilter) | std::ranges::views::split(',');
        // for (const auto &signal : signals) {
        //     requestedSignals.emplace(std::string_view(signal.begin(), signal.end()));
        // }
        // if (requestedSignals.empty()) {
        //     respondWithEmptyResponse(filterIn, "no signals requested, sending empty response\n");
        //     return false;
        // }

        // // check if signals exist
        // std::vector<std::string> unknownSignals;
        // for (const auto &requestedSignal : requestedSignals) {
        //     if (!_signalsMap.contains(requestedSignal)) {
        //         unknownSignals.push_back(requestedSignal);
        //     }
        // }
        // if (!unknownSignals.empty()) {
        //     respondWithEmptyResponse(filterIn, fmt::format("requested unknown signals: {}\n", unknownSignals));
        //     return false;
        // }

        return true;
    }

    void respondWithEmptyResponse(const TimeDomainContext &filter, const std::string_view errorText) {
        fmt::print("{}\n", errorText);
        super_t::notify("/Acquisition", filter, Acquisition());
    }
};

#endif /* TIME_DOMAIN_WORKER_H */
