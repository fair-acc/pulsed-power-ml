#include <majordomo/base64pp.hpp>
#include <majordomo/Broker.hpp>
#include <majordomo/RestBackend.hpp>

#include <atomic>
#include <fstream>
#include <iomanip>
#include <thread>

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

int main() {
    Broker                                          broker("Inference-Tool");
    auto                                            fs          = cmrc::assets::get_filesystem();
    const std::string_view                          REST_SCHEME = "https";
    const auto                                      REST_PORT   = 8081;
    opencmw::URI<>                                  restAddress = opencmw::URI<>::factory().scheme(REST_SCHEME).hostName("0.0.0.0").port(REST_PORT).build();

    FileServerRestBackend<PLAIN_HTTP, decltype(fs)> rest(broker, fs, "./", restAddress);

    const auto                                      brokerRouterAddress = broker.bind(opencmw::URI<>("mds://127.0.0.1:23456"));

    if (!brokerRouterAddress) {
        std::cerr << "Could not bind to broker address" << std::endl;
        return 1;
    }

    std::jthread brokerThread([&broker] { broker.run(); });

    // OpenCMW workers
    NilmPredictWorker<"nilm_predict_values", description<"Nilm Predicted Data">> nilmPredictWorker(broker, std::chrono::milliseconds(60));

    // run workers in separate threads
    std::jthread nilmPredictWorkerThread([&nilmPredictWorker] { nilmPredictWorker.run(); });

    brokerThread.join();

    // workers terminate when broker shuts down
    nilmPredictWorkerThread.join();
}