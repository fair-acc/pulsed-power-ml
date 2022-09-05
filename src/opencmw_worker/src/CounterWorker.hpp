#ifndef COUNTER_WORKER_H
#define COUNTER_WORKER_H

#include <majordomo/Worker.hpp>

struct TestContext {
    opencmw::TimingCtx      ctx;
    std::string             testFilter;
    opencmw::MIME::MimeType contentType = opencmw::MIME::BINARY;
};

ENABLE_REFLECTION_FOR(TestContext, ctx, testFilter, contentType)

struct CounterData {
    int         value;
    std::time_t timestamp;
};

ENABLE_REFLECTION_FOR(CounterData, value, timestamp)

using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class CounterWorker
    : public Worker<serviceName, TestContext, Empty, CounterData, Meta...> {
    std::atomic<bool>     shutdownRequested;
    std::jthread          notifyThread;
    int                   counter_value;
    std::time_t           timestamp;

    static constexpr auto PROPERTY_NAME = std::string_view("testCounter");

public:
    using super_t = Worker<serviceName, TestContext, Empty, CounterData, Meta...>;

    template<typename BrokerType>
    explicit CounterWorker(const BrokerType &broker,
            std::chrono::milliseconds        updateInterval)
        : super_t(broker, {}), counter_value(0) {
        notifyThread = std::jthread([this, updateInterval] {
            while (!shutdownRequested) {
                std::this_thread::sleep_for(updateInterval);
                if (counter_value < 100) {
                    counter_value++;
                } else {
                    counter_value = 0;
                }
                timestamp = std::time(nullptr);
                TestContext context;
                context.contentType = opencmw::MIME::JSON;
                CounterData reply;
                reply.value     = counter_value;
                reply.timestamp = timestamp;
                super_t::notify("/testCounter", context, reply);
            }
        });

        super_t::setCallback([this](RequestContext &rawCtx, const TestContext &,
                                     const Empty &, TestContext &,
                                     CounterData &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out.value     = counter_value;
            out.timestamp = timestamp;
        });
    }

    ~CounterWorker() {
        shutdownRequested = true;
        notifyThread.join();
    }
};

#endif /* COUNTER_WORKER_H */