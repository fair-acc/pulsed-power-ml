#ifndef NILM_PREDICT_WORKER_H
#define NILM_PREDICT_WORKER_H

#include <disruptor/RingBuffer.hpp>
#include <majordomo/Worker.hpp>

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>
#include <gnuradio/pulsed_power/opencmw_time_sink.h>

#include <chrono>
#include <unordered_map>

#include "integrator/PowerIntegrator.hpp"
#include <cppflow/cppflow.h>
#include <cppflow/model.h>
#include <cppflow/ops.h>
#include <cppflow/tensor.h>

#include "FrequencyDomainWorker.hpp"
#include "TimeDomainWorker.hpp"

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
    std::vector<std::string> names = { "device 1", "device 2", "device 3", "device 4", "device 5",
        "device 6", "device 7", "others" };
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

template<typename Acq>
class DataFetcher {
private:
    std::string     _endpoint;
    std::string     _signalNames;
    int64_t         _lastTimeStamp;
    httplib::Client _http;

public:
    DataFetcher() = delete;
    DataFetcher(const std::string &endPoint, const std::string &signalNames)
        : _endpoint(endPoint), _signalNames(signalNames), _lastTimeStamp(0), _http("localhost", DEFAULT_REST_PORT) {
        _http.set_keep_alive(true);
    }
    ~DataFetcher() {}
    httplib::Result fetch(Acq &data) {
        std::string getPath = fmt::format("{}?channelNameFilter={}&lastRefTrigger={}", _endpoint, _signalNames, _lastTimeStamp);
        // fmt::print("{}: path: {}\n", typeid(data).name(), getPath);
        auto response = _http.Get(getPath.data());
        if (response.error() == httplib::Error::Success && response->status == 200) {
            opencmw::IoBuffer buffer;
            buffer.put<opencmw::IoBuffer::MetaInfo::WITHOUT>(response->body);
            auto result = opencmw::deserialise<opencmw::Json, opencmw::ProtocolCheck::LENIENT>(buffer, data);
        }

        return response;
    }

    void updateTimeStamp(Acquisition &data) {
        if (!data.channelTimeSinceRefTrigger.empty()) {
            _lastTimeStamp = data.refTriggerStamp + static_cast<int64_t>(data.channelTimeSinceRefTrigger.back() * 1e9f);
        }
    }
};

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class NilmPredictWorker
    : public Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...> {
private:
    const std::string                MODEL_PATH      = "src/model/TFGuptaModel_v2-0";

    std::shared_ptr<cppflow::model>  _model          = std::make_shared<cppflow::model>(MODEL_PATH);

    std::shared_ptr<PQSPhiDataSink>  _pqsphiDataSink = std::make_shared<PQSPhiDataSink>();
    std::shared_ptr<SUIDataSink>     _suiDataSink    = std::make_shared<SUIDataSink>();

    bool                             init;
    int64_t                          timestamp_frq;

    std::mutex                       _nilmTimeSinksRegistryMutex;
    std::mutex                       _nilmFrequencySinksRegistryMutex;

    std::shared_ptr<PowerIntegrator> _powerIntegrator;

    DataFetcher<Acquisition>         _dataFetcherAcq        = DataFetcher<Acquisition>("pulsed_power/Acquisition", "P@100Hz,Q@100Hz,S@100Hz,phi@100Hz");
    DataFetcher<AcquisitionSpectra>  _dataFetcherAcqSpectra = DataFetcher<AcquisitionSpectra>("pulsed_power_freq/AcquisitionSpectra", "S@200000Hz");

public:
    std::atomic<bool>     _shutdownRequested;
    std::jthread          _predictThread;
    std::jthread          _fetchThread;
    NilmPredictData       _nilmData;
    std::time_t           _timestamp_n;

    static constexpr auto PROPERTY_NAME = std::string_view("NilmPredictData");

    using super_t                       = Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...>;

    template<typename BrokerType>
    explicit NilmPredictWorker(const BrokerType &broker,
            std::chrono::milliseconds            updateInterval)
        : super_t(broker, {}) {
        _powerIntegrator = std::make_shared<PowerIntegrator>(_nilmData.names.size(), "./src/data/", 1);

        _fetchThread     = std::jthread([this] {
            std::chrono::duration<double, std::milli> fetchDuration;
            while (!_shutdownRequested) {
                std::chrono::time_point time_start = std::chrono::system_clock::now();

                // fetch P,Q,S,phi from PulsedPowerService
                Acquisition acqPQSPhi;
                auto        response = _dataFetcherAcq.fetch(acqPQSPhi);
                _dataFetcherAcq.updateTimeStamp(acqPQSPhi);
                if (response.error() == httplib::Error::Success && response->status == 200) {
                    if (!acqPQSPhi.channelNames.empty()) {
                        uint32_t samplesNo         = acqPQSPhi.channelValues.n(1);

                        _pqsphiDataSink->timestamp = acqPQSPhi.refTriggerStamp;
                        _pqsphiDataSink->p         = acqPQSPhi.channelValues[{ 0U, samplesNo - 1 }];
                        _pqsphiDataSink->q         = acqPQSPhi.channelValues[{ 1U, samplesNo - 1 }];
                        _pqsphiDataSink->s         = acqPQSPhi.channelValues[{ 2U, samplesNo - 1 }];
                        _pqsphiDataSink->phi       = acqPQSPhi.channelValues[{ 3U, samplesNo - 1 }];
                    }
                }

                // fetch Apparent Power (S) from PulsedPowerService
                AcquisitionSpectra acqSData;
                response = _dataFetcherAcqSpectra.fetch(acqSData);
                if (response.error() == httplib::Error::Success && response->status == 200) {
                    // fmt::print("signal fft size: {}, signal_name: {}\n", acqSData.channelMagnitudeValues.size(), acqSData.channelName);
                    if (!acqSData.channelMagnitudeValues.empty()) {
                        _suiDataSink->timestamp = acqSData.refTriggerStamp;
                        _suiDataSink->s         = std::move(acqSData.channelMagnitudeValues);
                    }
                }

                fetchDuration     = std::chrono::system_clock::now() - time_start;
                auto willSleepFor = std::chrono::milliseconds(60) - fetchDuration;
                if (willSleepFor > 0ms) {
                    std::this_thread::sleep_for(willSleepFor);
                }
            }
            });

        _predictThread   = std::jthread([this, updateInterval] {
            std::chrono::duration<double, std::milli> predictDuration;
            while (!_shutdownRequested) {
                std::chrono::time_point time_start = std::chrono::system_clock::now();

                _timestamp_n                       = std::time(nullptr);

                NilmContext        context;
                std::vector<float> data_point;

                try {
                    _nilmData.timestamp = _timestamp_n;
                    _nilmData.day_usage.clear();
                    _nilmData.week_usage.clear();
                    _nilmData.month_usage.clear();

                    {
                        std::scoped_lock lock_time_nilm(_nilmTimeSinksRegistryMutex);
                        std::scoped_lock lock_freq_nilm(_nilmFrequencySinksRegistryMutex);

                        data_point                       = mergeValues();

                        int64_t         time_from_pqsphi = (*_pqsphiDataSink).timestamp;

                        int64_t         size             = static_cast<int64_t>(data_point.size());
                        cppflow::tensor input(data_point, { size });

                        auto            output = (*_model)({ { "serving_default_args_0:0", input } }, { "StatefulPartitionedCall:0" });

                        auto            values = output[0].get_data<float>();

                        // values print
                        fmt::print("input:  {}\n", input);
                        fmt::print("output: {}\n", output[0]);
                        fmt::print("output data: {}\n", values);

                        // fill data for REST
                        _nilmData.values.clear();
                        for (auto v : values) {
                            _nilmData.values.push_back(static_cast<double>(v));
                        }
                        _powerIntegrator->update(time_from_pqsphi, values);
                    }

                    fillDayUsage();
                    fillWeekUsage();
                    fillMonthUsage();

                } catch (const std::exception &ex) {
                    fmt::print("caught exception '{}'\n", ex.what());
                }

                super_t::notify("/nilmPredictData", context, _nilmData);

                predictDuration   = std::chrono::system_clock::now() - time_start;

                auto willSleepFor = updateInterval - predictDuration;

                if (willSleepFor > 0ms) {
                    std::this_thread::sleep_for(willSleepFor);
                } else {
                    fmt::print("Data prediction too slow\n");
                }
            }
          });

        super_t::setCallback([this](RequestContext &rawCtx, const NilmContext &,
                                     const Empty &, NilmContext &,
                                     NilmPredictData &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out = _nilmData;
        });
    }

    ~NilmPredictWorker() {
        _shutdownRequested = true;
        _predictThread.join();
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

            fmt::print("PQSPHI {} {} {} {}, timestamp {}\n", _pqsphiDataSink->p, _pqsphiDataSink->q, _pqsphiDataSink->s, _pqsphiDataSink->phi,
                    _pqsphiDataSink->timestamp);
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
    const size_t       fftSize = 65536;

    std::vector<float> mergeValues() {
        PQSPhiDataSink    &pqsphiData = *_pqsphiDataSink;
        SUIDataSink       &suiData    = *_suiDataSink;

        std::vector<float> output;

        if (suiData.u.size() == fftSize) {
            output.insert(output.end(), suiData.u.begin(), suiData.u.end());
        } else {
            fmt::print("Warning: incorrect u size {}\n", suiData.u.size());
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

    void fillDayUsage() {
        _nilmData.day_usage.clear();
        // dummy data
        // _nilmData.day_usage = { 100.0, 323.34, 234.33, 500.55, 100.0, 323.34, 234.33, 500.55, 100.0, 323.34, 234.33, 21.9 };
        auto power_day = _powerIntegrator->get_power_usages_day();
        for (auto v : power_day) {
            _nilmData.day_usage.push_back(v);
        }
    }

    void fillWeekUsage() {
        _nilmData.week_usage.clear();
        // dummy data
        // _nilmData.week_usage = { 700.0, 2123.34, 1434.33, 3500.55, 700.0, 2123.34, 1434.33, 3500.55, 700.0, 2123.34, 1434.33, 100.0 };
        auto power_week = _powerIntegrator->get_power_usages_week();
        for (auto v : power_week) {
            _nilmData.week_usage.push_back(v);
        }
    }

    void fillMonthUsage() {
        _nilmData.month_usage.clear();
        // dummy data
        // _nilmData.month_usage = { 1500.232, 3000.99, 2599.34, 1200.89, 1500.232, 3000.99, 2599.34, 1200.89, 1500.232, 3000.99, 2599.34, 300.0 };
        auto power_month = _powerIntegrator->get_power_usages_month();
        for (auto v : power_month) {
            _nilmData.month_usage.push_back(v);
        }
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
