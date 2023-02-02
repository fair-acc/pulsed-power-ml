#ifndef NILM_DATA_WORKER_H
#define NILM_DATA_WORKER_H

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

// context for InferenceTool
struct NilmDataContext {
    int64_t                 lastRefTrigger = 0;
    opencmw::MIME::MimeType contentType    = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(NilmDataContext, lastRefTrigger, contentType)

// data for InferenceTool
struct AcquisitionNilm {
    int64_t            refTriggerStamp = 0;
    size_t             fftSize         = 0;
    std::vector<float> voltageSpectrumStridedValues;       // fft(U)
    std::vector<float> currentSpectrumStridedValues;       // fft(I)
    std::vector<float> apparentPowerSpectrumStridedValues; // fft(S)
    std::vector<float> realPower;                          // P
    std::vector<float> reactivePower;                      // Q
    std::vector<float> apparentPower;                      // S
    std::vector<float> phi;
};

ENABLE_REFLECTION_FOR(AcquisitionNilm, refTriggerStamp, voltageSpectrumStridedValues, currentSpectrumStridedValues, apparentPowerSpectrumStridedValues, realPower, reactivePower, apparentPower, phi)

struct PQSPhiData {
    int64_t timestamp     = 0;
    float   realPower     = 0.0f; // P
    float   reactivePower = 0.0f; // Q
    float   apparentPower = 0.0f; // S
    float   phi           = 0.0f;
};

struct UISData {
    int64_t            timestamp = 0;
    std::vector<float> voltageSpectrum;
    std::vector<float> currentSpectrum;
    std::vector<float> apparentPowerSpectrum;
};

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class NilmDataWorker
    : public Worker<serviceName, NilmDataContext, Empty, AcquisitionNilm, Meta...> {
private:
    PQSPhiData          _timeData;
    std::mutex          _timeDataMutex;
    static const size_t RING_BUFFER_SIZE = 256;
    struct RingBufferData {
        size_t     fftSize = 0;
        UISData    freqData;
        PQSPhiData timeData;
    };
    using ringbuffer_t = std::shared_ptr<RingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;
    // using eventpoller_t = std::shared_ptr<EventPoller<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;
    using sequence_t = std::shared_ptr<Sequence>;
    ringbuffer_t _nilmDataBuffer;
    sequence_t   _nilmDataBufferTail;
    // eventpoller_t _nilmDataPoller;

public:
    using super_t = Worker<serviceName, NilmDataContext, Empty, AcquisitionNilm, Meta...>;
    std::atomic<bool> _shutdownRequested;

    template<typename BrokerType>
    explicit NilmDataWorker(const BrokerType &broker)
        : super_t(broker, {}), _nilmDataBuffer(newRingBuffer<RingBufferData, RING_BUFFER_SIZE, BusySpinWaitStrategy, ProducerType::Single>()), _nilmDataBufferTail(std::make_shared<Sequence>()) {
        _nilmDataBuffer->addGatingSequences({ _nilmDataBufferTail });

        // register callback only for "P Q S phi"
        std::scoped_lock lock_time(gr::pulsed_power::globalTimeSinksRegistryMutex);
        for (auto sink : gr::pulsed_power::globalTimeSinksRegistry) {
            const auto               signal_names = sink->get_signal_names();
            const auto               signal_units = sink->get_signal_units();
            const auto               sample_rate  = sink->get_sample_rate();

            std::vector<std::string> pqsohi_str{ "P", "Q", "S", "phi" };
            if (signal_names == pqsohi_str && sample_rate == 100.0f) {
                fmt::print("NilmDataWorker: name {}, unit {}, rate {}\n", signal_names, signal_units, sample_rate);
                sink->set_callback(std::bind(&NilmDataWorker::handleReceivedTimeDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
            }
        }

        // register callback only for Apparent Power Spectrum ("S")
        std::scoped_lock lock_freq(gr::pulsed_power::globalFrequencySinksRegistryMutex);
        for (auto sink : gr::pulsed_power::globalFrequencySinksRegistry) {
            const auto               signal_names = sink->get_signal_names();
            const auto               signal_units = sink->get_signal_units();
            const auto               sample_rate  = sink->get_sample_rate();

            std::vector<std::string> uis_str{ "ApparentPowerSpectrum" };
            if (signal_names == uis_str) {
                fmt::print("NilmDataWorker: name {}, unit {}, rate {}\n", signal_names, signal_units, sample_rate);
                sink->set_callback(std::bind(&NilmDataWorker::handleReceivedFreqDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
            }
        }

        super_t::setCallback([this](RequestContext &rawCtx, const NilmDataContext &requestContext,
                                     const Empty &, NilmDataContext &,
                                     AcquisitionNilm &out) {
            if (rawCtx.request.command() == Command::Get) {
                handleGetRequest(requestContext, out);
            }
        });
    }

    ~NilmDataWorker() {
        _shutdownRequested = true;
    }

    // copy P Q S Phi data
    void handleReceivedTimeDataCb(std::vector<const void *> &input_items, int &noutput_items, const std::vector<std::string> &signal_names, float /* sample_rate */, int64_t timestamp_ns) {
        const float             *realPower     = static_cast<const float *>(input_items[0]); // P
        const float             *reactivePower = static_cast<const float *>(input_items[1]); // Q
        const float             *apparentPower = static_cast<const float *>(input_items[2]); // S
        const float             *phi           = static_cast<const float *>(input_items[3]); // phi

        std::vector<std::string> pqsphi_str{ "P", "Q", "S", "phi" };
        if (signal_names == pqsphi_str) {
            int64_t timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            fmt::print("Sink {}, {}ms\n", pqsphi_str, timestamp_ms);

            std::scoped_lock lock(_timeDataMutex);
            // get last sample for each signal
            _timeData.timestamp     = timestamp_ns;
            _timeData.realPower     = realPower[noutput_items - 1];
            _timeData.reactivePower = reactivePower[noutput_items - 1];
            _timeData.apparentPower = apparentPower[noutput_items - 1];
            _timeData.phi           = phi[noutput_items - 1];
        }
    }

    // copy S U I data
    void handleReceivedFreqDataCb(std::vector<const void *> &input_items, int &nitems, size_t vector_size, const std::vector<std::string> &signal_names, float /*sample_rate*/, int64_t timestamp) {
        const float *s = static_cast<const float *>(input_items[0]);
        // const float             *u = static_cast<const float *>(input_items[1]);
        // const float             *i = static_cast<const float *>(input_items[2]);
        std::vector<std::string> sui_str{ "ApparentPowerSpectrum" };
        if (signal_names == sui_str) {
            using namespace std::chrono;
            int64_t timestamp_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            fmt::print("Sink {}, {}ms, vector_size: {}\n", sui_str, timestamp_ms, vector_size);

            for (int64_t k = 0; k < nitems; k++) {
                auto offset = k * static_cast<int64_t>(vector_size);

                bool result = _nilmDataBuffer->tryPublishEvent([&s, vector_size, offset, nitems, timestamp, this](RingBufferData &&bufferData, std::int64_t /*sequence*/) noexcept {
                    bufferData.fftSize = vector_size;
                    // bufferData.freqData.realPowerMagnitudeValues.assign(p + offset, s + offset + vector_size);
                    // bufferData.freqData.reactivePowerMagnitudeValues.assign(q + offset, s + offset + vector_size);
                    bufferData.freqData.apparentPowerSpectrum.assign(s + offset, s + offset + vector_size);
                    bufferData.freqData.timestamp = timestamp;

                    std::scoped_lock lock(_timeDataMutex);
                    bufferData.timeData = _timeData;
                });

                if (result) {
                    auto       headValue       = _nilmDataBuffer->cursor();
                    auto       tailValue       = _nilmDataBufferTail->value();

                    const auto tailOffsetValue = static_cast<int64_t>(RING_BUFFER_SIZE) * 50 / 100; // 50 %

                    if (headValue > (tailValue + tailOffsetValue)) {
                        _nilmDataBufferTail->setValue(headValue - tailOffsetValue);
                    }

                } else {
                    // error writing into RingBuffer
                    nitems = 0;
                }
            }
        }
    }

    bool handleGetRequest(const NilmDataContext &requestContext, AcquisitionNilm &out) {
        // if (_sinksMap.contains(requestContext.channelNameFilter)) {
        //     auto &sink = _sinksMap.at(requestContext.channelNameFilter);
        //     sink.fetchData(requestContext.lastRefTrigger, out);
        // } else {
        //     throw std::invalid_argument(fmt::format("Requested subscription for '{}' not found", requestContext.channelNameFilter));
        // }
        fetchNilmData(out);

        return true;
    }

    void fetchNilmData(AcquisitionNilm &out) {
        int64_t tail       = _nilmDataBufferTail->value();
        int64_t head       = _nilmDataBuffer->cursor();

        bool    firstChunk = true;
        int64_t sequence   = 0;
        for (sequence = tail; sequence <= head; sequence++) {
            const RingBufferData &bufData = (*_nilmDataBuffer)[sequence];

            if (firstChunk) {
                out.fftSize = bufData.fftSize;
                firstChunk  = false;
            }

            out.voltageSpectrumStridedValues.insert(out.voltageSpectrumStridedValues.end(), bufData.freqData.voltageSpectrum.begin(), bufData.freqData.voltageSpectrum.end());
            out.currentSpectrumStridedValues.insert(out.currentSpectrumStridedValues.end(), bufData.freqData.currentSpectrum.begin(), bufData.freqData.currentSpectrum.end());
            out.apparentPowerSpectrumStridedValues.insert(out.apparentPowerSpectrumStridedValues.end(), bufData.freqData.apparentPowerSpectrum.begin(), bufData.freqData.apparentPowerSpectrum.end());
            out.realPower.push_back(bufData.timeData.realPower);
            out.reactivePower.push_back(bufData.timeData.reactivePower);
            out.apparentPower.push_back(bufData.timeData.apparentPower);
            out.phi.push_back(bufData.timeData.phi);
        }

        if (!out.apparentPowerSpectrumStridedValues.empty()) {
            _nilmDataBufferTail->setValue(sequence);
        }

        // if (!stridedValues.empty()) {
        //     //  generate multiarray values from strided array
        //     size_t channelValuesSize = stridedValues.size() / _channelNames.size();
        //     out.channelValues        = opencmw::MultiArray<float, 2>(std::move(stridedValues), { static_cast<uint32_t>(_channelNames.size()), static_cast<uint32_t>(channelValuesSize) });
        //     //  generate relative timestamps
        //     out.channelTimeSinceRefTrigger.reserve(channelValuesSize);
        //     for (size_t i = 0; i < channelValuesSize; ++i) {
        //         float relativeTimestamp = static_cast<float>(i) / _sampleRate;
        //         out.channelTimeSinceRefTrigger.push_back(relativeTimestamp);
        //     }
        // } else {
        //     // throw std::invalid_argument(fmt::format("No new data available for signals: '{}'", _channelNames));
        // }
    };

private:
    const size_t fftSize = 65536;

    // std::vector<float> mergeValues() {
    //     PQSPhiDataSink    &pqsphiData = *_pqsphiDataSink;
    //     SUIDataSink       &suiData    = *_suiDataSink;

    //     std::vector<float> output;

    //     if (suiData.u.size() == fftSize) {
    //         output.insert(output.end(), suiData.u.begin(), suiData.u.end());
    //     } else {
    //         fmt::print("Warning: incorrect u size {}\n", suiData.u.size());
    //         output.insert(output.end(), suiData.u.begin(), suiData.u.end());
    //         if (suiData.u.size() < fftSize) {
    //             auto               size = fftSize - suiData.u.size();
    //             std::vector<float> suffix(size);
    //             std::fill(suffix.begin(), suffix.end(), 0);
    //             output.insert(output.end(), suffix.begin(), suffix.end());
    //         }
    //     }

    //     if (suiData.i.size() == fftSize) {
    //         output.insert(output.end(), suiData.i.begin(), suiData.i.end());
    //     } else {
    //         fmt::print("Warning: incorrect i size {}\n", suiData.i.size());

    //         output.insert(output.end(), suiData.i.begin(), suiData.i.end());
    //         if (suiData.i.size() < fftSize) {
    //             auto               size = fftSize - suiData.i.size();
    //             std::vector<float> suffix(size);
    //             std::fill(suffix.begin(), suffix.end(), 0);
    //             output.insert(output.end(), suffix.begin(), suffix.end());
    //         }
    //     }
    //     if (suiData.s.size() == fftSize) {
    //         output.insert(output.end(), suiData.s.begin(), suiData.s.end());
    //     } else {
    //         fmt::print("Warning: incorrect s size {}\n", suiData.s.size());
    //         output.insert(output.end(), suiData.s.begin(), suiData.s.end());

    //         // add 0 at the end
    //         if (suiData.s.size() < fftSize) {
    //             auto               size = fftSize - suiData.s.size();
    //             std::vector<float> suffix(size);
    //             std::fill(suffix.begin(), suffix.end(), 0);
    //             output.insert(output.end(), suffix.begin(), suffix.end());
    //         }
    //     }

    //     output.push_back(pqsphiData.p);
    //     output.push_back(pqsphiData.q);
    //     output.push_back(pqsphiData.s);
    //     output.push_back(pqsphiData.phi);

    //     return output;
    // }
};

#endif /* NILM_DATA_WORKER_H */
