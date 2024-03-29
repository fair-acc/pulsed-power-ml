#include <majordomo/Broker.hpp>
#include <majordomo/RestBackend.hpp>
#include <majordomo/Settings.hpp>
#include <majordomo/Worker.hpp>

#include <IoSerialiserJson.hpp>
#include <MIME.hpp>
#include <opencmw.hpp>

#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <refl.hpp>

#include <exception>

#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>
#include <gnuradio/top_block.h>

#include "helpers.hpp"
#include "TimeDomainWorker.hpp"

using opencmw::majordomo::Broker;
using opencmw::majordomo::BrokerMessage;
using opencmw::majordomo::Command;
using opencmw::majordomo::MdpMessage;
using opencmw::majordomo::MessageFrame;
using opencmw::majordomo::Settings;
using opencmw::majordomo::Worker;

using opencmw::majordomo::DEFAULT_REST_PORT;
using opencmw::majordomo::PLAIN_HTTP;

template<typename Mode, typename VirtualFS>
class SimpleTestRestBackend : public opencmw::majordomo::RestBackend<Mode, VirtualFS> {
    using super_t = opencmw::majordomo::RestBackend<Mode, VirtualFS>;

public:
    using super_t::RestBackend;

    static MdpMessage deserializeMessage(std::string_view method, std::string_view serialized) {
        // clang-format off
        auto result = MdpMessage::createClientMessage(
                method == "SUB" ? Command::Subscribe :
                method == "PUT" ? Command::Set :
                /* default */     Command::Get);
        // clang-format on

        // For the time being, just use ';' as frame separator. Not meant
        // to be a safe long-term solution:
        auto       currentBegin = serialized.cbegin();
        const auto bodyEnd      = serialized.cend();
        auto       currentEnd   = std::find(currentBegin, serialized.cend(), ';');

        for (std::size_t i = 2; i < result.requiredFrameCount(); ++i) {
            result.setFrameData(i, std::string_view(currentBegin, currentEnd), MessageFrame::dynamic_bytes_tag{});
            currentBegin = (currentEnd != bodyEnd) ? currentEnd + 1 : bodyEnd;
            currentEnd   = std::find(currentBegin, serialized.cend(), ';');
        }
        return result;
    }

    void registerHandlers() override {
        super_t::registerHandlers();
    }
};

std::jthread makeGetRequestResponseCheckerThread(const std::string &address, const std::string &requiredResponse, [[maybe_unused]] std::source_location location = std::source_location::current()) {
    return std::jthread([=] {
        httplib::Client http("localhost", DEFAULT_REST_PORT);
        http.set_keep_alive(true);
        const auto response = http.Get(address.data());

#define requireWithSource(arg) \
    if (!(arg)) opencmw::debug::withLocation(location) << "<- call got a failed requirement:"; \
    REQUIRE(arg)
        requireWithSource(response);
        requireWithSource(response->status == 200);
        requireWithSource(response->body.find(requiredResponse) != std::string::npos);
#undef requireWithSource
    });
}

class GRFlowgraph {
private:
    gr::top_block_sptr top;

public:
    GRFlowgraph(
            const double                   sampleRate,
            const double                   amplitude,
            const double                   frequency,
            const std::vector<std::string> signalName,
            const std::vector<std::string> signalUnit)
        : top(gr::make_top_block("GNURadio")) {
        // gnuradio blocks
        auto signalSource           = gr::analog::sig_source_f::make(sampleRate, gr::analog::GR_SAW_WAVE, frequency, amplitude, 0, 0);
        auto throttleBlock          = gr::blocks::throttle::make(sizeof(float) * 1, sampleRate, true);
        auto pulsedPowerOpencmwSink = gr::pulsed_power::opencmw_time_sink::make(signalName, signalUnit, static_cast<float>(sampleRate));
        pulsedPowerOpencmwSink->set_max_noutput_items(640);

        // connections
        // signal source --> throttle --> time sink
        top->hier_block2::connect(signalSource, 0, throttleBlock, 0);
        top->hier_block2::connect(throttleBlock, 0, pulsedPowerOpencmwSink, 0);
    }

    ~GRFlowgraph() { top->stop(); }

    void start() { top->start(); }
};

TEST_CASE("TimeDomainWorker service", "[daq_api][time-domain]") {
    const double      SAMPLING_RATE = 20'000.0;
    const double      AMPLITUDE     = 1.0;
    const double      FREQUENCY     = 50.0;
    const std::string signalName    = { "testSignal" };
    const std::string signalUnit    = { "testUnit" };

    GRFlowgraph       grFlowgraph(SAMPLING_RATE, AMPLITUDE, FREQUENCY, { signalName }, { signalUnit });
    grFlowgraph.start();

    // We run both broker and worker inproc
    Broker                                          broker("TestBroker");
    auto                                            fs = cmrc::assets::get_filesystem();
    SimpleTestRestBackend<PLAIN_HTTP, decltype(fs)> rest(broker, fs);

    // The worker uses the same settings for matching, but as it knows about TimeDomainContext, it does this registration automatically.
    opencmw::query::registerTypes(TimeDomainContext(), broker);

    TimeDomainWorker<"test.service", description<"Time-Domain Worker">> timeDomainWorker(broker);

    // Run worker and broker in separate threads
    RunInThread brokerRun(broker);
    RunInThread workerRun(timeDomainWorker);

    REQUIRE(waitUntilServiceAvailable(broker.context, "test.service"));

    httplib::Client http("localhost", DEFAULT_REST_PORT);
    http.set_keep_alive(true);

    const char *path = "test.service?channelNameFilter=testSignal@20000Hz";
    // httplib::Headers headers({ { "X-OPENCMW-METHOD", "POLL" } });
    auto response = http.Get(path);
    for (size_t i = 0; i < 100; i++) {
        response = http.Get(path);
        if (response.error() == httplib::Error::Success) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    REQUIRE(response.error() == httplib::Error::Success);
    REQUIRE(response->status == 200);

    REQUIRE(response->body.find("refTriggerName") != std::string::npos);
    REQUIRE(response->body.find("refTriggerStamp") != std::string::npos);
    REQUIRE(response->body.find("channelTimeSinceRefTrigger") != std::string::npos);
    REQUIRE(response->body.find("channelUserDelay") != std::string::npos);
    REQUIRE(response->body.find("channelActualDelay") != std::string::npos);
    REQUIRE(response->body.find("channelNames") != std::string::npos);
    REQUIRE(response->body.find("channelValues") != std::string::npos);
    REQUIRE(response->body.find("channelErrors") != std::string::npos);
    REQUIRE(response->body.find("channelUnits") != std::string::npos);
    REQUIRE(response->body.find("status") != std::string::npos);
    REQUIRE(response->body.find("channelRangeMin") != std::string::npos);
    REQUIRE(response->body.find("channelRangeMax") != std::string::npos);
    REQUIRE(response->body.find("temperature") != std::string::npos);

    {
        opencmw::IoBuffer buffer;
        buffer.put<opencmw::IoBuffer::MetaInfo::WITHOUT>(response->body);
        Acquisition data;
        auto        result = opencmw::deserialise<opencmw::Json, opencmw::ProtocolCheck::LENIENT>(buffer, data);
        fmt::print("deserialisation finished: {}\n", result);
        REQUIRE(data.refTriggerName == "NO_REF_TRIGGER");
        REQUIRE(data.refTriggerStamp == 0);
        REQUIRE(data.channelTimeSinceRefTrigger.size() == 0);
        REQUIRE(data.channelUserDelay == 0.0F);
        REQUIRE(data.channelActualDelay == 0.0F);
        REQUIRE(data.channelNames.size() == 0);
        REQUIRE(data.channelValues.n(0) == 0);
        REQUIRE(data.channelValues.n(1) == 0);
        REQUIRE(data.channelErrors.n(0) == 0);
        REQUIRE(data.channelErrors.n(1) == 0);
        REQUIRE(data.channelUnits.size() == 0);
        REQUIRE(data.status.size() == 0);
        REQUIRE(data.channelRangeMin.size() == 0);
        REQUIRE(data.channelRangeMax.size() == 0);
        REQUIRE(data.temperature.size() == 0);
    }
}

TEST_CASE("gr-opencmw_time_sink", "[daq_api][time-domain][opencmw_time_sink]") {
    const double      SAMPLING_RATE = 200'000.0;
    const double      SAW_AMPLITUDE = 1.0;
    const double      SAW_FREQUENCY = 50.0;
    const std::string signalName    = { "saw" };
    const std::string signalUnit    = { "unit" };

    GRFlowgraph       grFlowgraph(SAMPLING_RATE, SAW_AMPLITUDE, SAW_FREQUENCY, { signalName }, { signalUnit });
    grFlowgraph.start();

    // We run both broker and worker inproc
    Broker                                          broker("TestBroker");
    auto                                            fs = cmrc::assets::get_filesystem();
    SimpleTestRestBackend<PLAIN_HTTP, decltype(fs)> rest(broker, fs);

    // The worker uses the same settings for matching, but as it knows about TimeDomainContext, it does this registration automatically.
    opencmw::query::registerTypes(TimeDomainContext(), broker);

    TimeDomainWorker<"test.service", description<"Time-Domain Worker">> timeDomainWorker(broker);

    // Run worker and broker in separate threads
    RunInThread brokerRun(broker);
    RunInThread workerRun(timeDomainWorker);

    REQUIRE(waitUntilServiceAvailable(broker.context, "test.service"));

    httplib::Client http("localhost", DEFAULT_REST_PORT);
    http.set_keep_alive(true);

    const char *path = "test.service?channelNameFilter=saw@200000Hz";
    // httplib::Headers headers({ { "X-OPENCMW-METHOD", "POLL" } });
    auto response = http.Get(path);
    for (size_t i = 0; i < 100; i++) {
        response = http.Get(path);
        if (response.error() == httplib::Error::Success) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    REQUIRE(response.error() == httplib::Error::Success);
    REQUIRE(response->status == 200);

    {
        opencmw::IoBuffer buffer;
        buffer.put<opencmw::IoBuffer::MetaInfo::WITHOUT>(response->body);
        Acquisition data;
        auto        result = opencmw::deserialise<opencmw::Json, opencmw::ProtocolCheck::LENIENT>(buffer, data);
        fmt::print("deserialisation finished: {}\n", result);
        if (data.refTriggerStamp > 0) {
            REQUIRE(data.refTriggerStamp > 0);
            REQUIRE(data.channelTimeSinceRefTrigger.size() == data.channelValues.n(1));
            REQUIRE(data.channelNames.size() == data.channelValues.n(0));
            REQUIRE(data.channelNames[0] == fmt::format("{}@{}Hz", signalName, SAMPLING_RATE));

            // check if it is actually sawtooth signal
            for (uint32_t i = 0; i < data.channelValues.n(1) - 1; ++i) {
                Approx saw_signal_slope = Approx(SAW_AMPLITUDE / SAMPLING_RATE * SAW_FREQUENCY).epsilon(0.01); // 1% difference
                Approx saw_timebase     = Approx(1 / SAMPLING_RATE).epsilon(0.01);                             // 1% difference
                Approx saw_amplitude    = Approx(SAW_AMPLITUDE).epsilon(0.01);                                 // 1% difference
                if (data.channelValues[i + 1] > data.channelValues[i]) {
                    REQUIRE(saw_signal_slope == (data.channelValues[i + 1] - data.channelValues[i]));
                } else {
                    REQUIRE(data.channelValues[i] == saw_amplitude);
                }
                REQUIRE(saw_timebase == (data.channelTimeSinceRefTrigger[i + 1] - data.channelTimeSinceRefTrigger[i]));
            }
        }
    }
}

TEST_CASE("request_multiple_chunks_from_time_domain_worker", "[daq_api][time-domain][opencmw_time_sink]") {
    const double      SAMPLING_RATE = 200'000.0;
    const double      SAW_AMPLITUDE = 1.0;
    const double      SAW_FREQUENCY = 50.0;
    const std::string signalName{ "saw" };
    const std::string signalUnit{ "unit" };

    GRFlowgraph       grFlowgraph(SAMPLING_RATE, SAW_AMPLITUDE, SAW_FREQUENCY, { signalName }, { signalUnit });
    grFlowgraph.start();

    // We run both broker and worker inproc
    Broker                                          broker("TestBroker");
    auto                                            fs = cmrc::assets::get_filesystem();
    SimpleTestRestBackend<PLAIN_HTTP, decltype(fs)> rest(broker, fs);

    // The worker uses the same settings for matching, but as it knows about TimeDomainContext, it does this registration automatically.
    opencmw::query::registerTypes(TimeDomainContext(), broker);

    TimeDomainWorker<"test.service", description<"Time-Domain Worker">> timeDomainWorker(broker);

    // Run worker and broker in separate threads
    RunInThread brokerRun(broker);
    RunInThread workerRun(timeDomainWorker);

    REQUIRE(waitUntilServiceAvailable(broker.context, "test.service"));

    httplib::Client http("localhost", DEFAULT_REST_PORT);
    http.set_keep_alive(true);

    std::string path = "test.service?channelNameFilter=saw@200000Hz&lastRefTrigger=0";
    // httplib::Headers headers({ { "X-OPENCMW-METHOD", "GET" } });
    int64_t previousRefTrigger       = 0;
    int64_t lastTimeStamp            = 0;
    int64_t firstTimeStampOfNewChunk = 0;
    for (int iChunk = 0; iChunk < 100; iChunk++) {
        auto response = http.Get(path.c_str());
        for (size_t i = 0; i < 100; i++) {
            response = http.Get(path);
            if (response.error() == httplib::Error::Success && response->status == 200) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        REQUIRE(response.error() == httplib::Error::Success);
        REQUIRE(response->status == 200);

        opencmw::IoBuffer buffer;
        buffer.put<opencmw::IoBuffer::MetaInfo::WITHOUT>(response->body);
        Acquisition data;
        auto        result = opencmw::deserialise<opencmw::Json, opencmw::ProtocolCheck::LENIENT>(buffer, data);
        fmt::print("deserialisation finished: {}\n", result);

        // check for non-empty response
        if (data.refTriggerStamp == 0) {
            continue;
        }

        REQUIRE(data.channelTimeSinceRefTrigger.size() == data.channelValues.n(1));
        REQUIRE(data.channelNames.size() == data.channelValues.n(0));
        REQUIRE(data.channelNames[0] == fmt::format("{}@{}Hz", signalName, SAMPLING_RATE));

        // check if it is actually sawtooth signal
        for (uint32_t i = 0; i < data.channelValues.n(1) - 1; ++i) {
            Approx saw_signal_slope = Approx(SAW_AMPLITUDE / SAMPLING_RATE * SAW_FREQUENCY).epsilon(0.01); // 1% difference
            Approx saw_timebase     = Approx(1 / SAMPLING_RATE).epsilon(0.01);                             // 1% difference
            Approx saw_amplitude    = Approx(SAW_AMPLITUDE).epsilon(0.01);                                 // 1% difference
            if (data.channelValues[i + 1] > data.channelValues[i]) {
                REQUIRE(saw_signal_slope == (data.channelValues[i + 1] - data.channelValues[i]));
            } else {
                REQUIRE(data.channelValues[i] == saw_amplitude);
            }
            REQUIRE(saw_timebase == (data.channelTimeSinceRefTrigger[i + 1] - data.channelTimeSinceRefTrigger[i]));
        }

        // Check time-continuity between chunks
        if (previousRefTrigger != 0) {
            Approx calculatedRefTriggerStamp = Approx(static_cast<double>(lastTimeStamp) / 1.0 + SAMPLING_RATE).epsilon(1e-10);
            firstTimeStampOfNewChunk         = data.refTriggerStamp + static_cast<int64_t>(data.channelTimeSinceRefTrigger[0] * 1e9f);
            REQUIRE(firstTimeStampOfNewChunk == calculatedRefTriggerStamp);
            REQUIRE(firstTimeStampOfNewChunk > lastTimeStamp);
        }

        previousRefTrigger = data.refTriggerStamp;
        lastTimeStamp      = data.refTriggerStamp + static_cast<int64_t>(data.channelTimeSinceRefTrigger.back() * 1e9f);
        path               = fmt::format("test.service?channelNameFilter=saw@200000Hz&lastRefTrigger={}", lastTimeStamp);
    }
}
