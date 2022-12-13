#include <majordomo/base64pp.hpp>
#include <majordomo/Broker.hpp>
#include <majordomo/RestBackend.hpp>

// Gnu Radio includes
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/fft/fft.h>
#include <gnuradio/fft/fft_v.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>
#include <gnuradio/top_block.h>

#include <atomic>
#include <fstream>
#include <iomanip>
#include <thread>

#include "CounterWorker.hpp"
#include "FrequencyDomainWorker.hpp"
#include "NilmPowerWorker.hpp"
#include "NilmPredictWorker.hpp"
#include "TimeDomainWorker.hpp"

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
    GRFlowGraph(int noutput_items)
        : top(gr::make_top_block("GNURadio")) {
        // flowgraph setup
        float samp_rate = 4'000.0f;
        // sinus_signal --> throttle --> opencmw_time_sink
        auto signal_source_0             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SIN_WAVE, 0.5, 5, 0, 0);
        auto throttle_block_0            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_0 = gr::pulsed_power::opencmw_time_sink::make({ "sinus", "square" }, { "V", "A" }, samp_rate);
        pulsed_power_opencmw_sink_0->set_max_noutput_items(noutput_items);

        // saw_signal --> throttle --> opencmw_time_sink
        auto signal_source_1             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SAW_WAVE, 3, 4, 0, 0);
        auto throttle_block_1            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_1 = gr::pulsed_power::opencmw_time_sink::make({ "saw" }, { "A" }, samp_rate);
        pulsed_power_opencmw_sink_1->set_max_noutput_items(noutput_items);

        // square_signal --> throttle --> opencmw_time_sink
        auto signal_source_2             = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SQR_WAVE, 0.7, 3, 0, 0);
        auto throttle_block_2            = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate, true);
        auto pulsed_power_opencmw_sink_2 = gr::pulsed_power::opencmw_time_sink::make({ "square" }, { "A" }, samp_rate);
        pulsed_power_opencmw_sink_2->set_max_noutput_items(noutput_items);

        // sinus_signal --> throttle --> stream_to_vector --> fft --> fast_multiply_constant --> complex_to_mag^2 --> log10 --> opencmw_freq_sink
        const float  samp_rate_2                      = 32'000.0f;
        const size_t vec_length                       = 1024;
        const size_t fft_size                         = vec_length;
        const auto   bandwidth                        = samp_rate_2;
        auto         signal_source_3                  = gr::analog::sig_source_f::make(samp_rate_2, gr::analog::GR_SIN_WAVE, 3000.0f, 220.0);
        auto         throttle_block_3                 = gr::blocks::throttle::make(sizeof(float) * 1, samp_rate_2, true);
        auto         stream_to_vector_0               = gr::blocks::stream_to_vector::make(sizeof(float) * 1, vec_length);
        auto         fft_vxx_0                        = gr::fft::fft_v<float, true>::make(fft_size, gr::fft::window::blackmanharris(1024), true, 1);
        auto         multiply_const_xx_0              = gr::blocks::multiply_const_cc::make(1 / static_cast<float>(vec_length), vec_length);
        auto         complex_to_mag_squared_0         = gr::blocks::complex_to_mag_squared::make(vec_length);
        auto         nlog10_ff_0                      = gr::blocks::nlog10_ff::make(10, vec_length, 0);
        auto         pulsed_power_opencmw_freq_sink_0 = gr::pulsed_power::opencmw_freq_sink::make({ "sinus_fft" }, { "dB" }, samp_rate_2, bandwidth);

        // nilm worker (time and frequency sink)
        auto nilm_time_sink = gr::pulsed_power::opencmw_time_sink::make({ "P", "Q", "S", "Phi" }, { "W", "Var", "VA", "deg" }, samp_rate);
        nilm_time_sink->set_max_noutput_items(noutput_items);

        auto nilm_freq_sink = gr::pulsed_power::opencmw_freq_sink::make({ "S", "U", "I" }, { "dB", "dB", "dB" }, samp_rate, samp_rate);
        nilm_freq_sink->set_max_noutput_items(noutput_items);

        // connections
        // time-domain sinks
        top->hier_block2::connect(signal_source_0, 0, throttle_block_0, 0);
        top->hier_block2::connect(throttle_block_0, 0, pulsed_power_opencmw_sink_0, 0);

        top->hier_block2::connect(signal_source_1, 0, throttle_block_1, 0);
        top->hier_block2::connect(throttle_block_1, 0, pulsed_power_opencmw_sink_1, 0);

        top->hier_block2::connect(signal_source_2, 0, throttle_block_2, 0);
        top->hier_block2::connect(throttle_block_2, 0, pulsed_power_opencmw_sink_0, 1);

        // frequency-domain sinks
        top->hier_block2::connect(signal_source_3, 0, throttle_block_3, 0);
        top->hier_block2::connect(throttle_block_3, 0, stream_to_vector_0, 0);
        top->hier_block2::connect(stream_to_vector_0, 0, fft_vxx_0, 0);
        top->hier_block2::connect(fft_vxx_0, 0, multiply_const_xx_0, 0);
        top->hier_block2::connect(multiply_const_xx_0, 0, complex_to_mag_squared_0, 0);
        top->hier_block2::connect(complex_to_mag_squared_0, 0, nlog10_ff_0, 0);
        top->hier_block2::connect(nlog10_ff_0, 0, pulsed_power_opencmw_freq_sink_0, 0);

        // nilm worker (time and frequency sink)
        top->hier_block2::connect(throttle_block_0, 0, nilm_time_sink, 0);
        top->hier_block2::connect(throttle_block_1, 0, nilm_time_sink, 1);
        top->hier_block2::connect(throttle_block_2, 0, nilm_time_sink, 2);
        top->hier_block2::connect(throttle_block_3, 0, nilm_time_sink, 3);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 0);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 1);
        top->hier_block2::connect(nlog10_ff_0, 0, nilm_freq_sink, 2);
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
    GRFlowGraph flowgraph(1024);
    flowgraph.start();

    // OpenCMW workers
    CounterWorker<"counter", description<"Returns counter value">>                                        counterWorker(broker, std::chrono::milliseconds(1000));
    TimeDomainWorker<"pulsed_power/Acquisition", description<"Time-Domain Worker">>                       timeDomainWorker(broker);
    FrequencyDomainWorker<"pulsed_power_freq/AcquisitionSpectra", description<"Frequency-Domain Worker">> freqDomainWorker(broker);
    NilmPowerWorker<"nilm_values", description<"Nilm Data">>                                              nilmDataWorker(broker, std::chrono::milliseconds(1000));
    NilmPredictWorker<"nilm_predict_values", description<"Nilm Predicted Data">>                          nilmPredictWorker(broker, std::chrono::milliseconds(1000));

    // run workers in separate threads
    std::jthread counterWorkerThread([&counterWorker] { counterWorker.run(); });
    std::jthread freqSinkWorkerThread([&freqDomainWorker] { freqDomainWorker.run(); });
    std::jthread timeSinkWorkerThread([&timeDomainWorker] { timeDomainWorker.run(); });
    std::jthread nilmDataWorkerThread([&nilmDataWorker] { nilmDataWorker.run(); });
    std::jthread nilmPredictWorkerThread([&nilmPredictWorker] { nilmPredictWorker.run(); });

    brokerThread.join();

    // workers terminate when broker shuts down
    timeSinkWorkerThread.join();
    freqSinkWorkerThread.join();
    counterWorkerThread.join();
    nilmDataWorkerThread.join();
    nilmPredictWorkerThread.join();
}
