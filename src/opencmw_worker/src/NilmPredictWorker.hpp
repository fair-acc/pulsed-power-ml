#ifndef NILM_PREDICT_WORKER_H
#define NILM_PREDICT_WORKER_H

#include <majordomo/Worker.hpp>

#include "integrator/PowerIntegrator.hpp"
#include <cppflow/cppflow.h>
#include <cppflow/model.h>
#include <cppflow/ops.h>
#include <cppflow/tensor.h>

#include "NilmDataWorker.hpp"
// #include "TimeDomainWorker.hpp"

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
    std::vector<std::string> names = { "device 1", "device 2", "device 3", "device 4", "device 5", "device 6", "device 7", "others" };
    std::vector<double>      dayUsage;
    std::vector<double>      weekUsage;
    std::vector<double>      monthUsage;
};

ENABLE_REFLECTION_FOR(NilmPredictData, timestamp, values, names, dayUsage, weekUsage, monthUsage)

template<typename Acq>
class DataFetcher {
    std::string     _endpoint;
    std::string     _signalNames;
    int64_t         _lastTimeStamp;
    httplib::Client _http;

public:
    DataFetcher() = delete;
    DataFetcher(const std::string &endPoint, const std::string &signalNames = "")
        : _endpoint(endPoint), _signalNames(signalNames), _lastTimeStamp(0), _http("localhost", DEFAULT_REST_PORT) {
        _http.set_keep_alive(true);
    }
    ~DataFetcher() {}
    httplib::Result get(Acq &data) {
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

    // void updateTimeStamp(Acquisition &data) {
    //     if (!data.channelTimeSinceRefTrigger.empty()) {
    //         _lastTimeStamp = data.refTriggerStamp + static_cast<int64_t>(data.channelTimeSinceRefTrigger.back() * 1e9f);
    //     }
    // }
};

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class NilmPredictWorker : public Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...> {
    const std::string                MODEL_PATH{ "src/model/nilm_model" };
    cppflow::model                   _model{ MODEL_PATH };

    DataFetcher<AcquisitionNilm>     _acquisitionNilmFetcher{ "pulsed_power_nilm" };

    std::atomic<bool>                _shutdownRequested;
    std::jthread                     _predictThread;
    NilmPredictData                  _nilmData;
    std::shared_ptr<PowerIntegrator> _powerIntegrator = std::make_shared<PowerIntegrator>(_nilmData.names.size(), "./src/data/", 1);

public:
    using super_t = Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...>;

    template<typename BrokerType>
    explicit NilmPredictWorker(const BrokerType &broker, std::chrono::milliseconds updateInterval)
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
                std::chrono::time_point timeStart = std::chrono::system_clock::now();

                NilmContext             context;
                std::vector<float>      dataPoint;
                AcquisitionNilm         acquisitionNilm;

                try {
                    _nilmData.timestamp = std::time(nullptr);
                    _nilmData.dayUsage.clear();
                    _nilmData.weekUsage.clear();
                    _nilmData.monthUsage.clear();

                    // fetch AcquisitionNilm from PulsedPowerService
                    auto response = _acquisitionNilmFetcher.get(acquisitionNilm);

                    if (response.error() == httplib::Error::Success && response->status == 200) {
                        assert(!acquisitionNilm.apparentPowerSpectrumStridedValues.empty());
                        size_t fftSize = acquisitionNilm.apparentPowerSpectrumStridedValues.size() / acquisitionNilm.apparentPower.size();
                        fmt::print("acquisitionNilm received, chunks no: {},  fftsize: {}, size of data: {}\n", acquisitionNilm.apparentPower.size(), fftSize, response->body.size());
                        for (size_t i = 0; i < acquisitionNilm.realPower.size(); i++) {
                            mergeValues(acquisitionNilm, i, fftSize, dataPoint);

                            int64_t         size = static_cast<int64_t>(dataPoint.size());
                            cppflow::tensor input(dataPoint, { size });

                            auto            output = _model({ { "serving_default_args_0:0", input } }, { "StatefulPartitionedCall:0" });

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
                            _powerIntegrator->update(acquisitionNilm.refTriggerStamp[i], values);

                            super_t::notify("/nilmPredictData", context, _nilmData);
                        }
                    }

                    fillDayUsage();
                    fillWeekUsage();
                    fillMonthUsage();

                } catch (const std::exception &ex) {
                    fmt::print("caught exception '{}'\n", ex.what());
                }

                predictDuration   = std::chrono::system_clock::now() - timeStart;

                auto willSleepFor = updateInterval - predictDuration;

                if (willSleepFor > 0ms) {
                    std::this_thread::sleep_for(willSleepFor);
                } else {
                    fmt::print("Data prediction too slow\n");
                }
            }
        });

        super_t::setCallback([this](RequestContext &rawCtx, const NilmContext &, const Empty &, NilmContext &, NilmPredictData &out) {
            if (rawCtx.request.command() == Command::Get) {
                out = _nilmData;
            }
        });
    }

    ~NilmPredictWorker() {
        _shutdownRequested = true;
        _predictThread.join();
    }

private:
    void mergeValues(const AcquisitionNilm &acqNilmData, size_t i, size_t vectorSize, std::vector<float> &output) {
        // model requires only first half of the spectrum (2^16)
        size_t fftNilmSize = vectorSize / 2;
        output.clear();
        output.reserve(3 * fftNilmSize + 4);
        // voltage spectrum
        output.insert(output.end(), fftNilmSize, 0.0f);

        // current spectrum
        output.insert(output.end(), fftNilmSize, 0.0f);

        // apparent power spectrum
        if (acqNilmData.apparentPowerSpectrumStridedValues.empty()) {
            output.insert(output.end(), fftNilmSize, 0.0f);
        } else {
            int64_t offset = static_cast<int64_t>(i * vectorSize);
            output.insert(output.end(), acqNilmData.apparentPowerSpectrumStridedValues.begin() + offset, acqNilmData.apparentPowerSpectrumStridedValues.begin() + offset + static_cast<int64_t>(fftNilmSize));
        }

        // P Q S phi
        output.push_back(acqNilmData.realPower[i]);
        output.push_back(acqNilmData.reactivePower[i]);
        output.push_back(acqNilmData.apparentPower[i]);
        output.push_back(acqNilmData.phi[i]);
    }

    void fillDayUsage() {
        _nilmData.dayUsage.clear();
        // dummy data
        // _nilmData.dayUsage = { 100.0, 323.34, 234.33, 500.55, 100.0, 323.34, 234.33, 500.55, 100.0, 323.34, 234.33, 21.9 };
        auto powerDay = _powerIntegrator->get_power_usages_day();
        for (auto v : powerDay) {
            _nilmData.dayUsage.push_back(v);
        }
    }

    void fillWeekUsage() {
        _nilmData.weekUsage.clear();
        // dummy data
        // _nilmData.weekUsage = { 700.0, 2123.34, 1434.33, 3500.55, 700.0, 2123.34, 1434.33, 3500.55, 700.0, 2123.34, 1434.33, 100.0 };
        auto powerWeek = _powerIntegrator->get_power_usages_week();
        for (auto v : powerWeek) {
            _nilmData.weekUsage.push_back(v);
        }
    }

    void fillMonthUsage() {
        _nilmData.monthUsage.clear();
        // dummy data
        // _nilmData.monthUsage = { 1500.232, 3000.99, 2599.34, 1200.89, 1500.232, 3000.99, 2599.34, 1200.89, 1500.232, 3000.99, 2599.34, 300.0 };
        auto powerMonth = _powerIntegrator->get_power_usages_month();
        for (auto v : powerMonth) {
            _nilmData.monthUsage.push_back(v);
        }
    }
};

#endif /* NILM_PREDICT_WORKER_H */
