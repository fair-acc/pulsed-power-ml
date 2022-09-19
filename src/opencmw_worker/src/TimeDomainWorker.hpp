#ifndef TIME_DOMAIN_WORKER_H
#define TIME_DOMAIN_WORKER_H

#include <disruptor/RingBuffer.hpp>
#include <majordomo/Worker.hpp>

#include <gnuradio/pulsed_power/opencmw_time_sink.h>

#include <boost/functional/hash.hpp>
#include <chrono>
#include <unordered_map>

using opencmw::Annotated;
using opencmw::NoUnit;

struct TimeDomainContext {
    std::string             channelNameFilter;
    int32_t                 acquisitionModeFilter = 0;
    opencmw::MIME::MimeType contentType           = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(TimeDomainContext, channelNameFilter, acquisitionModeFilter, contentType)

struct Acquisition {
    int64_t                  refTriggerStamp;
    std::vector<float>       channelTimeSinceRefTrigger;
    std::vector<std::string> channelNames;
    std::vector<float>       channelValues; // TODO change this to MultiArray
    // opencmw::MultiArray<float, 2> channelValues2D;
};

ENABLE_REFLECTION_FOR(Acquisition, refTriggerStamp, channelTimeSinceRefTrigger, channelNames, channelValues)

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

    using pair_t = std::pair<std::string, float>;
    std::unordered_map<pair_t, SignalData, boost::hash<pair_t>> _signalsMap;

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

                for (auto &[key_pair, signal_data] : _signalsMap) {
                    bool firstEvent = true;
                    // poll data
                    PollState result = signal_data.eventPoller->poll([&](RingBufferData &event, std::int64_t /*sequence*/, bool /*nomoreEvts*/) noexcept {
                        if (firstEvent) {
                            _reply.refTriggerStamp = event.timestamp;
                            _reply.channelTimeSinceRefTrigger.clear();
                            _reply.channelNames.clear();
                            _reply.channelValues.clear();
                            _reply.channelNames.push_back(key_pair.first);
                            firstEvent = false;
                        }

                        _reply.channelValues.insert(_reply.channelValues.end(), event.chunk.begin(), event.chunk.end());

                        return true;
                    });

                    if (result == PollState::Processing) {
                        //  generate relative timestamps
                        for (int i = 0; i < _reply.channelValues.size(); ++i) {
                            float relative_timestamp = i * (1 / key_pair.second);
                            _reply.channelTimeSinceRefTrigger.push_back(relative_timestamp);
                        }
                        TimeDomainContext context;
                        context.contentType = opencmw::MIME::JSON;
                        super_t::notify(fmt::format("/timeDomainWorker/{}", key_pair.first), context, _reply);
                    }
                    pollingDuration = std::chrono::system_clock::now() - time_start;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(40) - pollingDuration);
            }
        });

        // map signal names and ringbuffers, register callback
        std::scoped_lock lock(gr::pulsed_power::globalTimeSinksRegistryMutex);
        for (auto sink : gr::pulsed_power::globalTimeSinksRegistry) {
            auto signal_name = sink->get_signal_name();
            auto signal_unit = sink->get_signal_unit();
            auto sample_rate = sink->get_sample_rate();

            // init RingBuffer, register poller and poller sequence
            auto ringbuffer = newRingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, ProducerType::Single>();
            auto poller     = ringbuffer->newPoller();
            ringbuffer->addGatingSequences({ poller->sequence() });

            _signalsMap[std::make_pair(signal_name, sample_rate)] = SignalData({ signal_name, signal_unit, sample_rate, ringbuffer, poller });

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
        if (_signalsMap.find(std::make_pair(signal_name, sample_rate)) != _signalsMap.end()) {
            const SignalData &signaldata = _signalsMap.at(std::make_pair(signal_name, sample_rate));

            // publish data
            bool result = signaldata.ringBuffer->tryPublishEvent([&data, data_size, timestamp_ns](RingBufferData &&bufferData, std::int64_t /*sequence*/) noexcept {
                bufferData.timestamp = timestamp_ns;
                bufferData.chunk.assign(data, data + data_size);
            });

            if (!result)
                fmt::print("error writing into RingBuffer, signal_name: {}\n", signal_name);
        }
    }
};

#endif /* TIME_DOMAIN_WORKER_H */
