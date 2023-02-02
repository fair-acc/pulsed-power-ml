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
#include "NilmDataWorker.hpp"
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

    DataFetcher<AcquisitionNilm>     _dataFetcherNilmSpectra = DataFetcher<AcquisitionNilm>("pulsed_power_nilm", "S@200000Hz");

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
                    // fmt::print("signal fft size: {}, signal_name: {}\n", acqSData.channelMagnitude_values.element_count(), acqSData.channelName);
                    if (acqSData.channelMagnitude_values.dimensions()[1] > 0) {
                        _suiDataSink->timestamp = acqSData.refTriggerStamp;
                        _suiDataSink->s.assign(acqSData.channelMagnitude_values.elements().end() - acqSData.channelMagnitude_values.dimensions()[1], acqSData.channelMagnitude_values.elements().end());
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

                    // fetch Apparent Power (S) from PulsedPowerService
                    AcquisitionNilm acqNilm;
                    auto            response = _dataFetcherNilmSpectra.fetch(acqNilm);
                    if (response.error() == httplib::Error::Success && response->status == 200) {
                        // fmt::print("signal fft size: {}, signal_name: {}\n", acqSData.channelMagnitudeValues.size(), acqSData.channelName);
                        if (!acqNilm.apparentPowerSpectrumStridedValues.empty()) {
                            fmt::print("acqNilm received, fftsize: {}, size of data: {}\n", acqNilm.apparentPowerSpectrumStridedValues.size(), response->body.size());
                        }
                        mergeValues(acqNilm, data_point);

                        int64_t         timestampFromResponse = acqNilm.refTriggerStamp;

                        int64_t         size                  = static_cast<int64_t>(data_point.size());
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
                        _powerIntegrator->update(timestampFromResponse, values);
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

private:
    const size_t fftSize = 65536;

    void         mergeValues(const AcquisitionNilm &nilmData, std::vector<float> &output) {
        if (nilmData.realPower.size() == 1) {
            // voltage spectrum
            if (nilmData.voltageSpectrumStridedValues.empty()) {
                output.insert(output.end(), fftSize, 0.0f);
            } else {
                output.insert(output.end(), nilmData.voltageSpectrumStridedValues.begin(), nilmData.voltageSpectrumStridedValues.begin() + fftSize);
            }

            // current spectrum
            if (nilmData.currentSpectrumStridedValues.empty()) {
                output.insert(output.end(), fftSize, 0.0f);
            } else {
                output.insert(output.end(), nilmData.currentSpectrumStridedValues.begin(), nilmData.currentSpectrumStridedValues.begin() + fftSize);
            }

            // apparent power spectrum
            if (nilmData.apparentPowerSpectrumStridedValues.empty()) {
                output.insert(output.end(), fftSize, 0.0f);
            } else {
                output.insert(output.end(), nilmData.apparentPowerSpectrumStridedValues.begin(), nilmData.apparentPowerSpectrumStridedValues.begin() + fftSize);
            }

            // P Q S phi
            output.push_back(nilmData.realPower[0]);
            output.push_back(nilmData.reactivePower[0]);
            output.push_back(nilmData.apparentPower[0]);
            output.push_back(nilmData.phi[0]);
        }
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
