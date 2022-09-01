#ifndef TIME_DOMAIN_WORKER_H
#define TIME_DOMAIN_WORKER_H

#include <majordomo/Worker.hpp>
#include <disruptor/RingBuffer.hpp>

#include <gnuradio/pulsed_power/opencmw_time_sink.h>

#include <chrono>

using opencmw::Annotated;
using opencmw::NoUnit;

struct TimeDomainContext {
    std::string             channelNameFilter;
    int32_t                 acquisitionModeFilter = 0;
    opencmw::MIME::MimeType contentType = opencmw::MIME::BINARY;
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
    int64_t            timebase;
};

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class TimeDomainWorker
    : public Worker<serviceName, TimeDomainContext, Empty, Acquisition, Meta...> {
private:
    static const int      RING_BUFFER_SIZE = 4096;
    std::atomic<bool>     _shutdownRequested;
    std::jthread          _pollingThread;
    Acquisition           _reply;

    std::shared_ptr<RingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>> _ringBuffer;
    std::shared_ptr<EventPoller<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>> _poller;
public:
    using super_t = Worker<serviceName, TimeDomainContext, Empty, Acquisition, Meta...>;

    template<typename BrokerType>
    explicit TimeDomainWorker(const BrokerType &broker)
        : super_t(broker, {})
    {
        // init RingBuffer, register poller and poller sequence
        _ringBuffer = newRingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, ProducerType::Single>();
        _poller = _ringBuffer->newPoller();
        _ringBuffer->addGatingSequences({ _poller-> sequence() });

        // polling thread
        _pollingThread = std::jthread([this] {
            while (!_shutdownRequested) {
                bool firstEvent = true;
                int64_t timebase_ns = 0;
                std::chrono::time_point time_start = std::chrono::system_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
                
                PollState result = _poller->poll([this, &firstEvent, &timebase_ns](RingBufferData &event, std::int64_t /*sequence*/, bool /*nomoreEvts*/) noexcept {
                    if (firstEvent) {
                        _reply.channelValues.clear();
                        _reply.channelTimeSinceRefTrigger.clear();
                        _reply.refTriggerStamp = event.timestamp;
                        timebase_ns = event.timebase;
                        firstEvent = false;
                    }

                    _reply.channelValues.insert(_reply.channelValues.end(), event.chunk.begin(), event.chunk.end());

                    return true;
                });

                if (result == PollState::Processing) {
                    fmt::print("polling done: channelValues size: {}\n", _reply.channelValues.size());
                    // generate relative timestamps
                    for (int i = 0; i < _reply.channelValues.size(); ++i) {
                        int64_t relative_timestamp = i * timebase_ns;
                        _reply.channelTimeSinceRefTrigger.push_back(static_cast<float>(relative_timestamp) / 1'000'000'000);
                    }
                }

                TimeDomainContext context;
                context.contentType = opencmw::MIME::JSON;;

                const std::chrono::duration<double, std::milli> time_elapsed = std::chrono::system_clock::now() - time_start;
                // fmt::print("polling result: {}, time elapsed: {} ms\n", result, time_elapsed.count());

                super_t::notify("/timeDomainWorker", context, _reply);
            }
        });

        // register callback
        std::scoped_lock lock(gr::pulsed_power::globalTimeSinksRegistryMutex);
        for (auto sink : gr::pulsed_power::globalTimeSinksRegistry) {
            sink->set_callback(std::bind(&TimeDomainWorker::callbackCopySinkData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        }

        super_t::setCallback([this](RequestContext &/*rawCtx*/, const TimeDomainContext &/*requestContext*/, const Empty &, 
                                                                      TimeDomainContext &/*replyContext*/, Acquisition &out) {
            out = _reply;
        });

    }

    ~TimeDomainWorker() {
        _shutdownRequested = true;
        _pollingThread.join();
    }

    void callbackCopySinkData(const float* data, int data_size, float sample_rate, int64_t timestamp_ns) {
        int64_t timebase_ns = 0;

        if (sample_rate > 0) {
            timebase_ns = static_cast<int64_t>(1 / sample_rate * 1e9F);
        }

        timestamp_ns -= timebase_ns * data_size;

        // write into RingBuffer
        // fmt::print("ringBuffer publishEvent, available space: {}\n", _ringBuffer->getRemainingCapacity());
        bool result = _ringBuffer->tryPublishEvent([&data, data_size, timestamp_ns, timebase_ns] (RingBufferData &&bufferData, std::int64_t /*sequence*/) noexcept {
            bufferData.timebase = timebase_ns;
            bufferData.timestamp = timestamp_ns;
            bufferData.chunk.assign(data, data + data_size);
        });

        if (!result)
            fmt::print("error writing into RingBuffer");
    }
};

#endif /* TIME_DOMAIN_WORKER_H */