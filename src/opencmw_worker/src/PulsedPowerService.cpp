#include <majordomo/base64pp.hpp>
#include <majordomo/Broker.hpp>
#include <majordomo/RestBackend.hpp>

// Gnu Radio includes
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>
#include <gnuradio/top_block.h>

#include <atomic>
#include <fstream>
#include <iomanip>
#include <thread>

#include "CounterWorker.hpp"
#include "TimeDomainWorker.hpp"
#include "NilmPowerWorker.hpp"
#include "NilmPredictWorker.hpp"

using namespace opencmw::majordomo;

CMRC_DECLARE(assets);

// from restserver_testapp.cpp
template<typename Mode, typename VirtualFS, role... Roles>
class FileServerRestBackend : public RestBackend<Mode, VirtualFS, Roles...> {
private:
    using super_t = RestBackend<Mode, VirtualFS, Roles...>;
    std::filesystem::path _serverRoot;
    using super_t::_svr;
    using super_t::DEFAULT_REST_SCHEME;

public:
    using super_t::RestBackend;

    FileServerRestBackend(Broker<Roles...> &broker, const VirtualFS &vfs, std::filesystem::path serverRoot, opencmw::URI<> restAddress = opencmw::URI<>::factory().scheme(DEFAULT_REST_SCHEME).hostName("0.0.0.0").port(DEFAULT_REST_PORT).build())
        : super_t(broker, vfs, restAddress), _serverRoot(std::move(serverRoot)) {
    }

    static auto deserializeSemicolonFormattedMessage(std::string_view method, std::string_view serialized) {
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
        _svr.set_mount_point("/", _serverRoot.string());

        _svr.Post("/stdio.html", [](const httplib::Request &request, httplib::Response &response) {
            opencmw::debug::log() << "QtWASM:" << request.body;
            response.set_content("", "text/plain");
        });

        auto cmrcHandler = [this](const httplib::Request &request, httplib::Response &response) {
            if (super_t::_vfs.is_file(request.path)) {
                auto file = super_t::_vfs.open(request.path);
                response.set_content(std::string(file.begin(), file.end()), "");
            }
        };

        _svr.Get("/assets/.*", cmrcHandler);

        // Register default handlers
        super_t::registerHandlers();
    }
};

class GRFlowGraph {
private:
    gr::top_block_sptr top;

public:
    GRFlowGraph(float samp_rate, int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // flowgraph setup
        // sinus_signal --> throttle --> opencmw_time_sink
        auto signal_source_0             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SIN_WAVE, 2, 5, 0, 0);
        auto throttle_block_0            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_0 = gr::pulsed_power::opencmw_time_sink::make(samp_rate, "sinus", "V");
        pulsed_power_opencmw_sink_0->set_max_noutput_items(noutput_items);

        // saw_signal --> throttle --> opencmw_time_sink
        auto signal_source_1             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SAW_WAVE, 3, 4, 0, 0);
        auto throttle_block_1            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_1 = gr::pulsed_power::opencmw_time_sink::make(samp_rate, "saw", "A");
        pulsed_power_opencmw_sink_1->set_max_noutput_items(noutput_items);

        // square_signal --> throttle --> opencmw_time_sink
        auto signal_source_2             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SQR_WAVE, 0.7, 3, 0, 0);
        auto throttle_block_2            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_2 = gr::pulsed_power::opencmw_time_sink::make(samp_rate, "square", "A");
        pulsed_power_opencmw_sink_2->set_max_noutput_items(noutput_items);

        // connections
        top->hier_block2::connect(signal_source_0, 0, throttle_block_0, 0);
        top->hier_block2::connect(throttle_block_0, 0, pulsed_power_opencmw_sink_0, 0);

        top->hier_block2::connect(signal_source_1, 0, throttle_block_1, 0);
        top->hier_block2::connect(throttle_block_1, 0, pulsed_power_opencmw_sink_1, 0);

        top->hier_block2::connect(signal_source_2, 0, throttle_block_2, 0);
        top->hier_block2::connect(throttle_block_2, 0, pulsed_power_opencmw_sink_2, 0);
    }

    ~GRFlowGraph() { top->stop(); }

    // start gnuradio flowgraph
    void start() { top->start(); }
};

int main() {
    Broker                                          broker("Pulsed-Power-Broker");
    auto                                            fs = cmrc::assets::get_filesystem();

    FileServerRestBackend<PLAIN_HTTP, decltype(fs)> rest(broker, fs, "./");

    const auto                                      brokerRouterAddress = broker.bind(opencmw::URI<>("mds://127.0.0.1:12345"));

    if (!brokerRouterAddress) {
        std::cerr << "Could not bind to broker address" << std::endl;
        return 1;
    }

    std::jthread brokerThread([&broker] { broker.run(); });

    // flowgraph setup
    GRFlowGraph flowgraph(4'000.0F, 80);
    flowgraph.start();

    // OpenCMW workers
    CounterWorker<"counter", description<"Returns counter value">>      counterWorker(broker, std::chrono::milliseconds(1000));
    TimeDomainWorker<"pulsed_power", description<"Time-Domain Worker">> timeDomainWorker(broker);
    NilmPowerWorker<"nilm_values", description<"Nilm Data">>            nilmDataWorker(broker, std::chrono::milliseconds(1000));
    NilmPredictWorker<"nilm_predict_values", description<"Nilm Predicted Data">>  nilmPredictWorker(broker, std::chrono::milliseconds(1000));


    // run workers in separate threads
    std::jthread counterWorkerThread([&counterWorker] { counterWorker.run(); });
    std::jthread timeSinkWorkerThread([&timeDomainWorker] { timeDomainWorker.run(); });
    std::jthread nilmDataWorkerThread([&nilmDataWorker] { nilmDataWorker.run(); });
    std::jthread nilmPredictWorkerThread([&nilmPredictWorker] { nilmPredictWorker.run(); });

    brokerThread.join();

    // workers terminate when broker shuts down
    timeSinkWorkerThread.join();
    counterWorkerThread.join();
    nilmDataWorkerThread.join();
    nilmPredictWorkerThread.join();

}
