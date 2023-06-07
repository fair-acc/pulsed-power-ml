#include <majordomo/base64pp.hpp>
#include <majordomo/Broker.hpp>
#include <majordomo/RestBackend.hpp>

#include <atomic>
#include <fstream>
#include <getopt.h>
#include <iomanip>
#include <thread>

#include "NilmPredictWorker.hpp"
#include "NilmWeekWorker.hpp"

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

int main(int argc, char *argv[]) {
    int                  opt;
    std::string          captureFilename;
    Mode                 mode          = Mode::Normal;
    const char          *shortOptions  = "r:w:h";
    static struct option longOptions[] = {
        { "read-from-file", required_argument, 0, 'r' },
        { "write-to-file", required_argument, 0, 'w' },
        { "help", no_argument, 0, 'h' },
        { 0, 0, 0, 0 }
    };

    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, shortOptions, longOptions, &optionIndex)) != -1) {
        switch (opt) {
        case 'r':
            mode            = Mode::Read;
            captureFilename = optarg;
            if (access(captureFilename.c_str(), F_OK) == -1) {
                fmt::print(std::cerr, "File {} does not exist\n", captureFilename);
                return 1;
            }
            fmt::print("Warning: Read mode is not currently supported, defaults to normal mode\n");
            break;
        case 'w':
            mode            = Mode::Write;
            captureFilename = optarg;
            break;

        case 'h':
            fmt::print("Usage: {} [-r /path/to/file] [-w /path/to/file] [-h]\n", argv[0]);
            fmt::print("Options:\n");
            fmt::print("  -r, --read-from-file    Read from file\n");
            fmt::print("  -w, --write-to-file     Write to file\n");
            fmt::print("  -h, --help              Display this help message\n");
            return 1;
            break;
        default:
            fmt::print(std::cerr, "Invalid command line option\n");
            return 1;
            break;
        }
    }

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
    NilmPredictWorker<"nilm_predict_values", description<"Nilm Predicted Data">> nilmPredictWorker(broker, std::chrono::milliseconds(60), mode, captureFilename);
    NilmWeekWorker<"nilm_week_values", description<"Nilm Week Data">> nilmWeekWorker(broker, std::chrono::milliseconds(1000));

    // run workers in separate threads
    std::jthread nilmPredictWorkerThread([&nilmPredictWorker] { nilmPredictWorker.run(); });
    std::jthread nilmWeekWorkerThread([&nilmWeekWorker] { nilmWeekWorker.run(); });

    brokerThread.join();

    // workers terminate when broker shuts down
    nilmPredictWorkerThread.join();
    nilmWeekWorkerThread.join();
}
