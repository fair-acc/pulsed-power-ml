#ifndef NILM_PREDICT_WORKER_H
#define NILM_PREDICT_WORKER_H

#include <disruptor/RingBuffer.hpp>
#include <majordomo/Worker.hpp>

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>

#include <chrono>
#include <unordered_map>

#include <cppflow/cppflow.h>
#include <cppflow/model.h>
#include <cppflow/ops.h>
#include <cppflow/tensor.h>

using opencmw::Annotated;
using opencmw::NoUnit;

// context for Dashboard
struct NilmContext {
    opencmw::TimingCtx      ctx;
    opencmw::MIME::MimeType contentType = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(NilmContext, ctx, contentType)

// data for Dashboard
struct NilmPredictData {
    int64_t                  timestamp;
    std::vector<double>      values; // = { 0.0, 25.7, 55.5, 74.1, 89.4, 34.5, 23.4, 1.0, 45.4, 56.5, 76.4, 23.8 };
    std::vector<std::string> names  = { "device 1", "device 2", "device 3", "device 4", "device 5",
         "device 6", "device 7", "device 8", "device 9", "device 10", "device 11", "others" };
    std::vector<double>      day_usage;
    std::vector<double>      week_usage;
    std::vector<double>      month_usage;
    std::string              error;
    int64_t                  error_code;
};

ENABLE_REFLECTION_FOR(NilmPredictData, values, names, timestamp, day_usage, week_usage, month_usage)

struct PQSPhiDataSink {
    int64_t timestamp;
    float   p;
    float   q;
    float   s;
    float   phi;
};

struct SUIDataSink {
    int64_t            timestamp;
    std::vector<float> u;
    std::vector<float> i;
    std::vector<float> s;
};

// input data for model
struct ModelData {
    int64_t            timestamp;
    std::vector<float> data_point; //(196612) = 4 + 2^16
};

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class NilmPredictWorker
    : public Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...> {
private:
    // const std::string  MODEL_PATH = "src/model/model_example";
    const std::string               MODEL_PATH      = "src/model/TFGuptaModel_v1-0";

    std::shared_ptr<cppflow::model> _model          = std::make_shared<cppflow::model>(MODEL_PATH);

    std::shared_ptr<PQSPhiDataSink> _pqsphiDataSink = std::make_shared<PQSPhiDataSink>();
    std::shared_ptr<SUIDataSink>    _suiDataSink    = std::make_shared<SUIDataSink>();

    bool                            init;
    int64_t                         timestamp_frq;

    std::mutex                      nilmTimeSinksRegistryMutex;
    std::mutex                      nilmFrequencySinksRegistryMutex;

public:
    std::atomic<bool>     shutdownRequested;
    std::jthread          notifyThread;
    NilmPredictData       nilmData;
    std::time_t           timestamp_n;

    static constexpr auto PROPERTY_NAME = std::string_view("NilmPredicData");

    using super_t                       = Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...>;

    template<typename BrokerType>
    explicit NilmPredictWorker(const BrokerType &broker,
            std::chrono::milliseconds            updateInterval)
        : super_t(broker, {}) {
        notifyThread = std::jthread([this, updateInterval] {
            std::chrono::duration<double, std::milli> pollingDuration;
            while (!shutdownRequested) {
                std::chrono::time_point time_start = std::chrono::system_clock::now();

                timestamp_n                        = std::time(nullptr);

                NilmContext        context;
                std::vector<float> data_point;

                try {
                    nilmData.timestamp = timestamp_n;
                    //nilmData.values.clear();
                    nilmData.day_usage.clear();
                    nilmData.week_usage.clear();
                    nilmData.month_usage.clear();

                    {
                        std::scoped_lock lock_time_nilm(nilmTimeSinksRegistryMutex);
                        std::scoped_lock lock_freq_nilm(nilmFrequencySinksRegistryMutex);

                        data_point = mergeValues();

                        fmt::print("Size of data point {}\n", data_point.size());
                    }

                    int64_t         size = static_cast<int64_t>(data_point.size());
                    cppflow::tensor input(data_point, { size });

                    //fmt::print("tensor print: {}\n", tensor);

                    fmt::print("input:  {}\n", input);

                    auto output = (*_model)({ { "serving_default_args_0:0", input } }, { "StatefulPartitionedCall:0" });

                    //int64_t size = static_cast<int64_t>(data_point.size());
                 //   cppflow::tensor tensor(data_point, { size });
                    // fmt::print("Tensor: {}\n", tensor);

                    auto values = output[0].get_data<float>();

                    // values print
                    fmt::print("output: {}\n", output[0]);

                    // fill data for REST
                    nilmData.values.clear();


                     // fill data for REST
                    for (auto v : values) {
                        nilmData.values.push_back(static_cast<double>(v));
                    } 

                    //fmt::print("Output: {}\n", output);

                    fillDayUsage();
                    fillWeekUsage();
                    fillMonthUsage();
                
                }
                 catch (const std::exception &ex) {
                    fmt::print("caught exception '{}'\n", ex.what()); 
                }
             
           /*      {//nilmData.values filled with dummy values                 

                    // generierte dummy Werte
                    for (std::size_t i=0;i<nilmData.names.size();i++){
                        double value_new = nilmData.values.at(i) + 25.5;
                        nilmData.values.at(i) = value_new<=100?value_new :  (value_new -100.0);
                    }
                    size_t zero_index = static_cast<size_t>( timestamp_n % 11);
                    nilmData.values.at(zero_index) = 0.0;
                } */

                super_t::notify("/nilmPredictData", context, nilmData);

                pollingDuration   = std::chrono::system_clock::now() - time_start;

                // TODO - check this - warning
                auto willSleepFor = updateInterval - pollingDuration;
                // if (willSleepFor>0){
                //     std::this_thread::sleep_for(willSleepFor);
                // } else {
                //     fmt::print("Data prediction too slow\n");
                // }

                //std::this_thread::sleep_for(1000ms);

                //std::this_thread::sleep_for(willSleepFor);
		
		if (willSleepFor > 0ms) {
                    std::this_thread::sleep_for(willSleepFor);
                } else {
                    fmt::print("Data prediction too slow\n");
                }
            }
        });

        std::scoped_lock lock(gr::pulsed_power::globalTimeSinksRegistryMutex);
        fmt::print("GR: number of time-domain sinks found: {}\n", gr::pulsed_power::globalTimeSinksRegistry.size());

        std::vector<std::string> pqsohi_str{ "P", "Q", "S", "Phi" };
        // from sink take only P Q S Phi Signal
        for (auto sink : gr::pulsed_power::globalTimeSinksRegistry) {
            const auto signal_names = sink->get_signal_names();
            const auto signal_units = sink->get_signal_units();
            const auto sample_rate  = sink->get_sample_rate();

            fmt::print("Name {}, unit {}, rate {}\n", signal_names, signal_units, sample_rate);

            //std::vector<std::string> pqsohi_str{ "P", "Q", "S", "Phi" };
            if (signal_names == pqsohi_str) {
                sink->set_callback(std::bind(&NilmPredictWorker::callbackCopySinkTimeData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
            }
        }

        std::scoped_lock         lock_freq(gr::pulsed_power::globalFrequencySinksRegistryMutex);
        std::vector<std::string> sui_str{ "S", "U", "I" };
        for (auto sink : gr::pulsed_power::globalFrequencySinksRegistry) {
            const auto signal_names = sink->get_signal_names();
            const auto signal_units = sink->get_signal_units();
            const auto sample_rate  = sink->get_sample_rate();

            //std::vector<std::string> sui_str{ "S", "U", "I" };
            if (signal_names == sui_str) {
                fmt::print("Name {}, unit {}, rate {}\n", signal_names, signal_units, sample_rate);
                sink->set_callback(std::bind(&NilmPredictWorker::callbackCopySinkFrequencyData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
            }
        }

        super_t::setCallback([this](RequestContext &rawCtx, const NilmContext &,
                                     const Empty &, NilmContext &,
                                     NilmPredictData &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out = nilmData;
        });
    }

    ~NilmPredictWorker() {
        shutdownRequested = true;
        notifyThread.join();
    }

    // copy P Q S Phi data
    void callbackCopySinkTimeData(std::vector<const void *> &input_items, int &noutput_items, const std::vector<std::string> &signal_names, float /* sample_rate */, int64_t timestamp_ns) {
        const float             *p   = static_cast<const float *>(input_items[0]);
        const float             *q   = static_cast<const float *>(input_items[1]);
        const float             *s   = static_cast<const float *>(input_items[2]);
        const float             *phi = static_cast<const float *>(input_items[3]);

        std::vector<std::string> pqsphi_str{ "P", "Q", "S", "Phi" };
        if (signal_names == pqsphi_str) {
            fmt::print("Sink {}\n", pqsphi_str);

            _pqsphiDataSink->timestamp = timestamp_ns;
            _pqsphiDataSink->p         = *(p + noutput_items - 1);
            _pqsphiDataSink->q         = *(q + noutput_items - 1);
            _pqsphiDataSink->s         = *(s + noutput_items - 1);
            _pqsphiDataSink->phi       = *(phi + noutput_items - 1);

            fmt::print("PQSPHI {} {} {} {}\n", _pqsphiDataSink->p, _pqsphiDataSink->q, _pqsphiDataSink->s, _pqsphiDataSink->phi);
        }
    }

    // copy S U I data
    void callbackCopySinkFrequencyData(std::vector<const void *> &input_items, int &nitems, size_t vector_size, const std::vector<std::string> &signal_name, float /*sample_rate*/, int64_t timestamp) {
        const float             *s = static_cast<const float *>(input_items[0]);
        const float             *u = static_cast<const float *>(input_items[1]);
        const float             *i = static_cast<const float *>(input_items[2]);

        std::vector<std::string> sui_str{ "S", "U", "I" };
        if (signal_name == sui_str) {
            fmt::print("Sink {}\n", sui_str);

            _suiDataSink->timestamp = timestamp;

            for (int64_t k = 0; k < nitems; k++) {
                auto offset = k * static_cast<int64_t>(vector_size);

                _suiDataSink->s.assign(s + offset, s + offset + vector_size);
                _suiDataSink->u.assign(u + offset, u + offset + vector_size);
                _suiDataSink->i.assign(i + offset, i + offset + vector_size);
            }
        }
    }

private:
    size_t             fftSize = 65536;

    std::vector<float> mergeValues() {
        PQSPhiDataSink    &pqsphiData = *_pqsphiDataSink;
        SUIDataSink       &suiData    = *_suiDataSink;

        std::vector<float> output;
        if (suiData.s.size() == fftSize) {
            output.insert(output.end(), suiData.s.begin(), suiData.s.end());
        } else {
            fmt::print("Warning: incorrect s size {}\n", suiData.s.size());
            // output.clear();
            // return output;
            output.insert(output.end(), suiData.s.begin(), suiData.s.end());
            // add 0 at the end
            if (suiData.s.size() < fftSize) {
                auto               size = fftSize - suiData.s.size();
                std::vector<float> suffix(size);
                std::fill(suffix.begin(), suffix.end(), 0);
                output.insert(output.end(), suffix.begin(), suffix.end());
            }
        }

        if (suiData.u.size() == fftSize) {
            output.insert(output.end(), suiData.u.begin(), suiData.u.end());
        } else {
            fmt::print("Warning: incorrect u size {}\n", suiData.u.size());
            // output.clear();
            // return output;
            output.insert(output.end(), suiData.u.begin(), suiData.u.end());
            if (suiData.u.size() < fftSize) {
                auto               size = fftSize - suiData.u.size();
                std::vector<float> suffix(size);
                std::fill(suffix.begin(), suffix.end(), 0);
                output.insert(output.end(), suffix.begin(), suffix.end());
            }
        }

        if (suiData.i.size() == fftSize) {
            output.insert(output.end(), suiData.i.begin(), suiData.i.end());
        } else {
            fmt::print("Warning: incorrect i size {}\n", suiData.i.size());
            // output.clear();
            // return output;

            output.insert(output.end(), suiData.i.begin(), suiData.i.end());
            if (suiData.i.size() < fftSize) {
                auto               size = fftSize - suiData.i.size();
                std::vector<float> suffix(size);
                std::fill(suffix.begin(), suffix.end(), 0);
                output.insert(output.end(), suffix.begin(), suffix.end());
            }
        }
        if (suiData.s.size() == fftSize) {
            output.insert(output.end(), suiData.s.begin(), suiData.s.end());
        } else {
            fmt::print("Warning: incorrect s size {}\n", suiData.s.size());
            // output.clear();
            // return output;
            output.insert(output.end(), suiData.s.begin(), suiData.s.end());
            // add 0 at the end
            if (suiData.s.size() < fftSize) {
                auto               size = fftSize - suiData.s.size();
                std::vector<float> suffix(size);
                std::fill(suffix.begin(), suffix.end(), 0);
                output.insert(output.end(), suffix.begin(), suffix.end());
            }
        }

        output.push_back(pqsphiData.p);
        output.push_back(pqsphiData.q);
        output.push_back(pqsphiData.s);
        output.push_back(pqsphiData.phi);

        return output;
    }

    void fillDayUsage(){
        nilmData.day_usage.clear();
        // dummy data
        nilmData.day_usage = { 100.0,323.34,234.33, 500.55, 100.0,323.34,234.33, 500.55, 100.0,323.34,234.33, 21.9};
    }

    void fillWeekUsage(){
        nilmData.week_usage.clear();
        //dummy data
        nilmData.week_usage = { 700.0, 2123.34, 1434.33, 3500.55, 700.0, 2123.34, 1434.33, 3500.55,700.0,2123.34,1434.33,100.0};
    }

    void fillMonthUsage(){
        nilmData.month_usage.clear();
        //dummy data
        nilmData.month_usage =  {1500.232, 3000.99, 2599.34, 1200.89, 1500.232, 3000.99, 2599.34, 1200.89,1500.232, 3000.99, 2599.34 ,300.0};

    }

    // Test - dummy vector
    std::vector<float> generateVector(float init_f) {
        std::vector<float> v;
        for (int i = 0; i < fftSize; i++) {
            v.push_back(i * init_f);
        }

        return v;
    }
};

#endif /* NILM_PREDICT_WORKER_H */
