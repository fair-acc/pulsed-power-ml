#include <majordomo/base64pp.hpp>
#include <majordomo/Broker.hpp>
#include <majordomo/RestBackend.hpp>
#include <majordomo/Worker.hpp>

#include <disruptor/RingBuffer.hpp>

#include <units/isq/si/voltage.h>

// Gnu Radio includes
#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/pulsed_power/opencmw_sink.h>

#include <atomic>
#include <fstream>
#include <iomanip>
#include <thread>

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

struct TestContext {
    opencmw::TimingCtx      ctx;
    std::string             testFilter;
    opencmw::MIME::MimeType contentType = opencmw::MIME::BINARY;
};

// TODO using unsupported types throws in the mustache serialiser, the exception isn't properly handled,
// the browser just shows a bit of gibberish instead of the error message.

ENABLE_REFLECTION_FOR(TestContext, ctx, testFilter, contentType)

struct HelloRequest {
    std::string             name;
    opencmw::TimingCtx      timingCtx;
    std::string             customFilter;
    opencmw::MIME::MimeType contentType = opencmw::MIME::BINARY;
};

ENABLE_REFLECTION_FOR(HelloRequest, name, timingCtx, customFilter /*, contentType*/)

struct HelloReply {
    std::string        name;
    bool               booleanReturnType;
    int8_t             byteReturnType;
    int16_t            shortReturnType;
    int32_t            intReturnType;
    int64_t            longReturnType;
    std::string        byteArray;
    opencmw::TimingCtx timingCtx;
    std::string        lsaContext;
    // Option replyOption = Option::REPLY_OPTION2;
};

ENABLE_REFLECTION_FOR(HelloReply, name, booleanReturnType, byteReturnType, shortReturnType, intReturnType, longReturnType, timingCtx, lsaContext /*, replyOption*/)

struct HelloWorldHandler {
    std::string customFilter = "uninitialised";

    void        operator()(RequestContext &rawCtx, const TestContext &requestContext, const HelloRequest &in, TestContext &replyContext, HelloReply &out) {
        using namespace std::chrono;
        const auto now        = system_clock::now();
        const auto sinceEpoch = system_clock::to_time_t(now);
        out.name              = fmt::format("Hello World! The local time is: {}", std::put_time(std::localtime(&sinceEpoch), "%Y-%m-%d %H:%M:%S"));
        out.byteArray         = in.name; // doesn't really make sense atm
        out.byteReturnType    = 42;

        out.timingCtx         = opencmw::TimingCtx(3, {}, {}, {}, duration_cast<microseconds>(now.time_since_epoch()));
        if (rawCtx.request.command() == Command::Set) {
            customFilter = in.customFilter;
        }
        out.lsaContext           = customFilter;

        replyContext.ctx         = out.timingCtx;
        replyContext.ctx         = opencmw::TimingCtx(3, {}, {}, {}, duration_cast<microseconds>(now.time_since_epoch()));
        replyContext.contentType = requestContext.contentType;
        replyContext.testFilter  = fmt::format("HelloWorld - reply topic = {}", requestContext.testFilter);
    }
};

struct CounterData {
    int         value;
    std::time_t timestamp;
};

ENABLE_REFLECTION_FOR(CounterData, value, timestamp)

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
                }
                else {
                    counter_value = 0;
                } 
                timestamp     = std::time(nullptr);
                TestContext context;
                context.contentType = opencmw::MIME::JSON;
                CounterData reply;
                reply.value     = counter_value;
                reply.timestamp = timestamp;
                super_t::notify("/", context, reply);
            }
        });

        super_t::setCallback([this](RequestContext &rawCtx, const TestContext &,
                                     const Empty &, TestContext &,
                                     CounterData &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out.value       = counter_value;
            out.timestamp   = timestamp;
        });
    }

    ~CounterWorker() {
        shutdownRequested = true;
        notifyThread.join();
    }
};


struct Acquisition {
    std::string        channelName;
    std::vector<float> channelValue;
    std::time_t        acqLocalTimeStamp;
};

ENABLE_REFLECTION_FOR(Acquisition, channelName, channelValue, acqLocalTimeStamp)

using opencmw::majordomo::Empty;

template<units::basic_fixed_string serviceName, typename... Meta>
class TimeDomainWorker
    : public Worker<serviceName, TestContext, Empty, Acquisition, Meta...> {
private:
    static constexpr auto PROPERTY_NAME = std::string_view("timeDomainWorker");
    Acquisition _reply;

public:
    using super_t = Worker<serviceName, TestContext, Empty, Acquisition, Meta...>;

    void callbackCopyData(const float* data, int data_size) {
        // TODO write into RingBuffer
        std::cout << __func__ << " received data size: " << data_size << std::endl;

        _reply.channelName = "Sinus Signal";
        _reply.channelValue.assign(data, data + data_size);
        _reply.acqLocalTimeStamp = std::time(nullptr);

        TestContext context;
        context.contentType = opencmw::MIME::JSON;

        super_t::notify("/", context, _reply);
    }

    template<typename BrokerType>
    explicit TimeDomainWorker(const BrokerType &broker,
                      std::chrono::milliseconds updateInterval)
        : super_t(broker, {}) 
    {
        std::scoped_lock lock(gr::pulsed_power::globalSinksRegistryMutex);
        for (auto sink : gr::pulsed_power::globalSinksRegistry) {
            sink->set_callback(std::bind(&TimeDomainWorker::callbackCopyData, this, std::placeholders::_1, std::placeholders::_2));
        }

        super_t::setCallback([this](RequestContext &/*rawCtx*/, const TestContext &, const Empty &, 
                                                                      TestContext &, Acquisition &out) {
            out = _reply;
        });
    }

    ~TimeDomainWorker() {}

    
};

int main() {
    using opencmw::URI;

    Broker primaryBroker("PrimaryBroker");
    auto   fs = cmrc::assets::get_filesystem();

    FileServerRestBackend<PLAIN_HTTP, decltype(fs)> rest(primaryBroker, fs, "./");

    const auto brokerRouterAddress = primaryBroker.bind(URI<>("mds://127.0.0.1:12345"));

    if (!brokerRouterAddress) {
        std::cerr << "Could not bind to broker address" << std::endl;
        return 1;
    }

    // note: our thread handling is very verbose, offer nicer API
    std::jthread primaryBrokerThread([&primaryBroker] {
        primaryBroker.run();
    });

    // top block
    auto top = gr::make_top_block("GNURadio");

    // sampling rate
    int samp_rate = 4000;

    // gnuradio blocks
    // sinus_signal --> throttle --> opencmw_sink
    auto sinus_signal_source = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_SIN_WAVE, 50, 1, 0,0);
    auto throttle_block = gr::blocks::throttle::make(sizeof(float)*1, samp_rate, true);
    auto pulsed_power_opencmw_sink = gr::pulsed_power::opencmw_sink::make();

    // connections
    top->hier_block2::connect(sinus_signal_source, 0, throttle_block, 0);
    top->hier_block2::connect(throttle_block, 0, pulsed_power_opencmw_sink, 0);
    
    // start gnuradio flowgraph
    top->start();

    // opencmw workers
    Worker<"helloWorld", TestContext, HelloRequest, HelloReply, description<"A friendly service saying hello">> helloWorldWorker(primaryBroker, HelloWorldHandler());
    CounterWorker<"testCounter", description<"Returns counter value">>    counterWorker(primaryBroker, std::chrono::milliseconds(1000));
    TimeDomainWorker<"timeDomainSink", description<"Time-Domain Worker">> timeDomainWorker(primaryBroker, std::chrono::milliseconds(1000));

    std::jthread helloWorldThread([&helloWorldWorker] {
        helloWorldWorker.run();
    });

     std::jthread counterWorkerThread([&counterWorker] {
         counterWorker.run();
     });

    std::jthread timeSinkWorkerThread([&timeDomainWorker] {
        timeDomainWorker.run();
    });


    primaryBrokerThread.join();

    // workers terminate when broker shuts down
    top->stop();
    helloWorldThread.join();
    counterWorkerThread.join();
    timeSinkWorkerThread.join();
}
