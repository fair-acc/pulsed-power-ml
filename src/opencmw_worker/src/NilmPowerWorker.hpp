#ifndef NILM_WORKER_H
#define NILM_WORKER_H

#include <majordomo/Worker.hpp>

// struct TestContext {
//     opencmw::TimingCtx      ctx;
//     std::string             testFilter;
//     opencmw::MIME::MimeType contentType = opencmw::MIME::BINARY;
// };

//ENABLE_REFLECTION_FOR(TestContext, ctx, testFilter, contentType)

struct NilmPowerData {
    std::vector<double>  values={0.0,25.7,55.5,74.1, 89.4, 34.5,23.4,1.0, 45.4, 56.5,76.4};
};

ENABLE_REFLECTION_FOR(NilmPowerData, values)

using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class NilmPowerWorker   
    : public Worker<serviceName, TestContext, Empty, NilmPowerData, Meta...> {
    std::atomic<bool>     shutdownRequested;
    std::jthread          notifyThread;
    NilmPowerData         dummy_data;
    std::time_t           timestamp;
    std::size_t           counter = 0;
    std::size_t           zero_index = 0;

    static constexpr auto PROPERTY_NAME = std::string_view("NilmPowerData");

public:
    using super_t = Worker<serviceName, TestContext, Empty, NilmPowerData, Meta...>;
 
    template<typename BrokerType>
    explicit NilmPowerWorker(const BrokerType &broker,
            std::chrono::milliseconds        updateInterval)
        : super_t(broker, {}) {
        notifyThread = std::jthread([this, updateInterval] {
            while (!shutdownRequested) {
                std::this_thread::sleep_for(updateInterval);
                timestamp = std::time(nullptr);
                TestContext context;
                context.contentType = opencmw::MIME::JSON;
                if (counter == 5){
                    counter = 0;
                    for (std::size_t i=0;i<11;i++){
                        double value_new = dummy_data.values.at(i) + 25.5;
                        dummy_data.values.at(i) = value_new<=100?value_new :  (value_new -100.0);
                    }
                    dummy_data.values.at(zero_index) = 0.0;
                    zero_index = zero_index == 10? 0: (zero_index +1);
                }
                counter++;
                super_t::notify("/nilmData", context, dummy_data);
            }
        });

        super_t::setCallback([this](RequestContext &rawCtx, const TestContext &,
                                     const Empty &, TestContext &,
                                     NilmPowerData &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out     = dummy_data;

        });
    }

    ~NilmPowerWorker() {
        shutdownRequested = true;
        notifyThread.join();
    }
};

#endif /* NILM_WORKER_H */