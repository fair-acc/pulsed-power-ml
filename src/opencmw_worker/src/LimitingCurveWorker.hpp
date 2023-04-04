#ifndef LIMITING_CURVE_WORKER_H
#define LIMITING_CURVE_WORKER_H

#include <majordomo/Worker.hpp>
#include <MultiArray.hpp>

struct TestContext {
    opencmw::TimingCtx      ctx;
    std::string             testFilter;
    opencmw::MIME::MimeType contentType = opencmw::MIME::BINARY;
};

ENABLE_REFLECTION_FOR(TestContext, ctx, testFilter, contentType)

class LimitingCurve {
public:
    std::string                   refTriggerName  = { "NO_REF_TRIGGER" };
    int64_t                       refTriggerStamp = -1; // time not relevant
    std::vector<float>            channelTimeSinceRefTrigger;
    std::string                   channelName = "limiting_curve";
    opencmw::MultiArray<float, 2> channelMagnitude_values;
    std::string                   channelMagnitude_unit;
    std::vector<long>             channelMagnitude_dim1_discrete_time_values;
    std::vector<float>            channelMagnitude_dim2_discrete_freq_values;
    opencmw::MultiArray<float, 2> channelPhase_values;
    std::string                   channelPhase_unit;
    std::vector<long>             channelPhase_dim1_discrete_time_values;
    std::vector<float>            channelPhase_dim2_discrete_freq_values;

    LimitingCurve() {
        std::vector<float> channelMagnitudeValuesArray = { 15, 15, 10, 10, 25, 25, 20, 20, 30, 30, 25, 15, 10, 10 };
        channelMagnitude_values                        = opencmw::MultiArray<float, 2>(std::move(channelMagnitudeValuesArray), { static_cast<uint32_t>(1), static_cast<uint32_t>(14) });
        channelMagnitude_dim2_discrete_freq_values     = { 0.0f, 0.05f, 0.1f, 0.3f, 0.4f, 0.8f, 0.85f, 2.6f, 2.65f, 4.4f, 4.8f, 5.2f, 5.6f, 6.8f };
    }
};

ENABLE_REFLECTION_FOR(LimitingCurve, refTriggerName, refTriggerStamp, channelTimeSinceRefTrigger, channelName, channelMagnitude_values, channelMagnitude_unit, channelMagnitude_dim1_discrete_time_values, channelMagnitude_dim2_discrete_freq_values, channelPhase_values, channelPhase_unit, channelPhase_dim1_discrete_time_values, channelPhase_dim2_discrete_freq_values)

using namespace opencmw::majordomo;
template<units::basic_fixed_string ServiceName, typename... Meta>
class LimitingCurveWorker
    : public Worker<ServiceName, TestContext, Empty, LimitingCurve, Meta...> {
    std::atomic<bool> _shutdownRequested;
    std::jthread      _pollingThread;
    // std::chrono::duration<std::chrono::milliseconds> _updateInterval = std::chrono::milliseconds(40);
    LimitingCurve         _limitingCurve;
    static constexpr auto PROPERTY_NAME = std::string_view("testLimitingCurve");

public:
    using super_t = Worker<ServiceName, TestContext, Empty, LimitingCurve, Meta...>;

    template<typename BrokerType>
    explicit LimitingCurveWorker(const BrokerType &broker)
        : super_t(broker, {}) {
        /*_pollingThread = std::jthread([this] {
            while (!_shutdownRequested) {
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
                TestContext context;
                context.contentType = opencmw::MIME::JSON;
                LimitingCurve reply = _limitingCurve;
                super_t::notify("limitingCurve", context, reply);
            }
        });*/
        super_t::setCallback([this](RequestContext &rawCtx, const TestContext &,
                                     const Empty &, TestContext &,
                                     LimitingCurve &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out = _limitingCurve;
        });
    }

    ~LimitingCurveWorker() {
        _shutdownRequested = true;
        _pollingThread.join();
    }
};

#endif /* LIMITING_CURVE_WORKER_H */
