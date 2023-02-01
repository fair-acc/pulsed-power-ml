#ifndef LIMITING_CURVE_WORKER_H
#define LIMITING_CURVE_WORKER_H

#include <majordomo/Worker.hpp>

class LimitingCurve {
public:
    std::string        refTriggerName         = { "NO_REF_TRIGGER" };
    int64_t            refTriggerStamp        = -1; // time not relevant
    std::string        channelName            = "limiting_curve";
    std::vector<float> channelMagnitudeValues = {};
    std::vector<float> channelFrequencyValues = {};
    std::string        channelUnit;

    LimitingCurve() {
        channelMagnitudeValues = { 15, 15, 10, 10, 25, 25, 20, 20, 30, 30, 25, 15, 10, 10 };
        channelFrequencyValues = { 0, 0.05, 0.1, 0.3, 0.4, 0.8, 0.85, 2.6, 2.65, 4.4, 4.8, 5.2, 5.6, 6.8 };
        // channelFrequencyValues = { 0, 0.05 * 1000, 0.1 * 1000, 0.3 * 1000, 0.4 * 1000, 0.8 * 1000, 0.85 * 1000, 2.6 * 1000, 2.65 * 1000, 4.4 * 1000, 4.8 * 1000, 5.2 * 1000, 5.6 * 1000, 6.8 * 1000 };
    }
};

ENABLE_REFLECTION_FOR(LimitingCurve, refTriggerName, refTriggerStamp, channelName, channelMagnitudeValues, channelFrequencyValues, channelUnit)

using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class LimitingCurveWorker
    : public Worker<serviceName, TestContext, Empty, LimitingCurve, Meta...> {
    std::atomic<bool>     shutdownRequested;
    std::jthread          notifyThread;
    LimitingCurve         limitingCurve;

    static constexpr auto PROPERTY_NAME = std::string_view("testLimitingCurve");

public:
    using super_t = Worker<serviceName, TestContext, Empty, LimitingCurve, Meta...>;

    template<typename BrokerType>
    explicit LimitingCurveWorker(const BrokerType &broker,
            std::chrono::milliseconds              updateInterval)
        : super_t(broker, {}) {
        notifyThread = std::jthread([this, updateInterval] {
            while (!shutdownRequested) {
                std::this_thread::sleep_for(updateInterval);
                TestContext context;
                context.contentType = opencmw::MIME::JSON;
                LimitingCurve reply = limitingCurve;
                super_t::notify("limitingCurve", context, reply);
            }
        });

        super_t::setCallback([this](RequestContext &rawCtx, const TestContext &,
                                     const Empty &, TestContext &,
                                     LimitingCurve &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out = limitingCurve;
        });
    }

    ~LimitingCurveWorker() {
        shutdownRequested = true;
        notifyThread.join();
    }
};

#endif /* LIMITING_CURVE_WORKER_H */
