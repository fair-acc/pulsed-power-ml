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

TEST_CASE("TimeDomainWorker service", "[daq_api][time-domain]") {
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

    const auto response = http.Get("test.service/timeDomainWorker", httplib::Headers({ { "X-OPENCMW-METHOD", "POLL" } }));

    REQUIRE(response->status == 200);

    REQUIRE(response->body.find("Acquisition") != std::string::npos);
    REQUIRE(response->body.find("refTriggerStamp") != std::string::npos);
    REQUIRE(response->body.find("channelTimeSinceRefTrigger") != std::string::npos);
    REQUIRE(response->body.find("channelNames") != std::string::npos);
    REQUIRE(response->body.find("channelValues") != std::string::npos);
}

TEST_CASE("gr-opencmw_time_sink", "[daq_api][time-domain][opencmw_time_sink]") {
    // top block
    auto top = gr::make_top_block("GNURadio");

    // sampling rate
    float samp_rate = 200'000;

    // gnuradio blocks
    // sinus_signal --> throttle --> opencmw_time_sink
    auto sinus_signal_source       = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SAW_WAVE, 50, 1, 0, 0);
    auto throttle_block            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
    auto pulsed_power_opencmw_sink = gr::pulsed_power::opencmw_time_sink::make(samp_rate, "Saw Tooth Signal", "units");
    pulsed_power_opencmw_sink->set_max_noutput_items(640);

    // connections
    top->hier_block2::connect(sinus_signal_source, 0, throttle_block, 0);
    top->hier_block2::connect(throttle_block, 0, pulsed_power_opencmw_sink, 0);

    // start gnuradio flowgraph
    top->start();

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

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    httplib::Client http("localhost", DEFAULT_REST_PORT);
    http.set_keep_alive(true);

    const auto response = http.Get("test.service/timeDomainWorker", httplib::Headers({ { "X-OPENCMW-METHOD", "POLL" } }));

    REQUIRE(response->status == 200);

    REQUIRE(response->body.find("Acquisition") != std::string::npos);
    REQUIRE(response->body.find("refTriggerStamp") != std::string::npos);
    REQUIRE(response->body.find("channelTimeSinceRefTrigger") != std::string::npos);
    REQUIRE(response->body.find("channelNames") != std::string::npos);
    REQUIRE(response->body.find("channelValues") != std::string::npos);

    {
        opencmw::IoBuffer buffer;
        buffer.put<opencmw::IoBuffer::MetaInfo::WITHOUT>(response->body);
        Acquisition data;
        auto        result = opencmw::deserialise<opencmw::Json, opencmw::ProtocolCheck::LENIENT>(buffer, data);
        fmt::print("deserialisation finished: {}\n", result);
        REQUIRE(data.channelTimeSinceRefTrigger.size() == data.channelValues.size());
    }

    top->stop();
}