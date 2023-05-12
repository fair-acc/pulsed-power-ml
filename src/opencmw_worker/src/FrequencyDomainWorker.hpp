#ifndef FREQUENCY_DOMAIN_WORKER_H
#define FREQUENCY_DOMAIN_WORKER_H

#define BOOST_BIND_NO_PLACEHOLDERS

#include "Ringbuffer.hpp"
#include <majordomo/Worker.hpp>

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>

#include <chrono>
#include <unordered_map>

#include <iostream>

using opencmw::Annotated;
using opencmw::NoUnit;

struct FreqDomainContext {
    std::string             channelNameFilter;
    int32_t                 acquisitionModeFilter = 0; // STREAMING
    std::string             triggerNameFilter;
    int32_t                 maxClientUpdateFrequencyFilter = 25;
    int64_t                 lastRefTrigger                 = 0;
    opencmw::MIME::MimeType contentType                    = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(FreqDomainContext, channelNameFilter, acquisitionModeFilter, triggerNameFilter, maxClientUpdateFrequencyFilter, lastRefTrigger, contentType)

struct AcquisitionSpectra {
    std::string                   refTriggerName  = { "NO_REF_TRIGGER" };
    int64_t                       refTriggerStamp = 0;
    std::vector<float>            channelTimeSinceRefTrigger;
    std::string                   channelName;
    opencmw::MultiArray<float, 2> channelMagnitude_values;
    std::string                   channelMagnitude_unit;
    std::vector<long>             channelMagnitude_dim1_discrete_time_values;
    std::vector<float>            channelMagnitude_dim2_discrete_freq_values;
    opencmw::MultiArray<float, 2> channelPhase_values;
    std::string                   channelPhase_unit;
    std::vector<long>             channelPhase_dim1_discrete_time_values;
    std::vector<float>            channelPhase_dim2_discrete_freq_values;
};

ENABLE_REFLECTION_FOR(AcquisitionSpectra, refTriggerName, refTriggerStamp, channelTimeSinceRefTrigger, channelName, channelMagnitude_values, channelMagnitude_unit, channelMagnitude_dim1_discrete_time_values, channelMagnitude_dim2_discrete_freq_values, channelPhase_values, channelPhase_unit, channelPhase_dim1_discrete_time_values, channelPhase_dim2_discrete_freq_values)

using namespace opencmw::majordomo;
template<units::basic_fixed_string ServiceName, typename... Meta>
class FrequencyDomainWorker
    : public Worker<ServiceName, FreqDomainContext, Empty, AcquisitionSpectra, Meta...> {
private:
    const size_t      RING_BUFFER_SIZE = 512;
    const std::string _deviceName;
    // std::atomic<bool>  _shutdownRequested;
    // std::jthread       _pollingThread;
    AcquisitionSpectra _reply;
    struct RingBufferData {
        std::vector<float> chunk;
        int64_t            timestamp = 0;
    };
    using ringbuffer_t = std::shared_ptr<Ringbuffer<RingBufferData>>;
    struct SignalData {
        gr::pulsed_power::opencmw_freq_sink *sink = nullptr;
        ringbuffer_t                         ringBuffer;
    };

    std::unordered_map<std::string, SignalData> _signalsMap; // <completeSignalName, signalData>

public:
    using super_t = Worker<ServiceName, FreqDomainContext, Empty, AcquisitionSpectra, Meta...>;

    template<typename BrokerType>
    explicit FrequencyDomainWorker(const BrokerType &broker)
        : super_t(broker, {}) {
        // polling thread
        /*_pollingThread = std::jthread([this] {
            auto pollingDuration = std::chrono::duration<double, std::milli>();
            while (!_shutdownRequested) {
                std::chrono::time_point timeStart = std::chrono::system_clock::now();

                for (auto subTopic : super_t::activeSubscriptions()) { // loop over active subscriptions
                    if (subTopic.path() != "/AcquisitionSpectra") {
                        break;
                    }
                    const auto              queryMap        = subTopic.queryParamMap();
                    const FreqDomainContext filterIn        = opencmw::query::deserialise<FreqDomainContext>(queryMap);
                    std::string             requestedSignal = filterIn.channelNameFilter;
                    if (_signalsMap.contains(requestedSignal)) {
                        bool result = pollSignal(requestedSignal, filterIn.lastRefTrigger, _reply);
                        if (!result) {
                            _reply = AcquisitionSpectra();
                        }
                    } else {
                        _reply = AcquisitionSpectra();
                    }

                    FreqDomainContext filterOut = filterIn;
                    filterOut.contentType       = opencmw::MIME::JSON;
                    super_t::notify("/AcquisitionSpectra", filterOut, _reply);
                }
                pollingDuration   = std::chrono::system_clock::now() - timeStart;

                auto willSleepFor = std::chrono::milliseconds(40) - pollingDuration;
                std::this_thread::sleep_for(willSleepFor);
            }
        });*/
        // map signal names and ringbuffers, register callback
        std::scoped_lock lock(gr::pulsed_power::globalFrequencySinksRegistryMutex);
        fmt::print("GR: number of frequency-domain sinks found: {}\n", gr::pulsed_power::globalFrequencySinksRegistry.size());
        for (gr::pulsed_power::opencmw_freq_sink *sink : gr::pulsed_power::globalFrequencySinksRegistry) {
            const auto signalNames = sink->get_signal_names();
            const auto sampleRate  = sink->get_sample_rate();

            // init RingBuffer and name for siganl (only one signal possible per freq_sink)
            auto       ringbuffer         = std::make_shared<Ringbuffer<RingBufferData>>(RING_BUFFER_SIZE);
            const auto completeSignalName = fmt::format("{}@{}Hz", signalNames[0], sampleRate);
            _signalsMap.insert({ completeSignalName, SignalData(sink, ringbuffer) });
            fmt::print("GR: OpenCMW Frequency Sink '{}' added\n", completeSignalName);

            // register callback
            sink->set_callback(std::bind(&FrequencyDomainWorker::callbackCopySinkData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
        }

        super_t::setCallback([this](RequestContext &rawCtx, const FreqDomainContext            &requestContext, const Empty &,
                                     FreqDomainContext /* &replyContext */, AcquisitionSpectra &out) {
            if (rawCtx.request.command() == Command::Get) {
                handleGetRequest(requestContext, out);
            }
        });
    }

    ~FrequencyDomainWorker() = default;
    /*{    _shutdownRequested = true;
        _pollingThread.join();
    } */

    void callbackCopySinkData(std::vector<const void *> &input_items, int &nitems, size_t vector_size, const std::vector<std::string> &signal_name, float sample_rate, int64_t timestamp) {
        const float *in                 = static_cast<const float *>(input_items[0]);
        const auto   completeSignalName = fmt::format("{}@{}Hz", signal_name[0], sample_rate);
        if (_signalsMap.contains(completeSignalName)) {
            const SignalData &signalData = _signalsMap.at(completeSignalName);

            for (int i = 0; i < nitems; i++) {
                // publish data
                RingBufferData bufferData;
                bufferData.timestamp = timestamp + (static_cast<int64_t>((static_cast<float>(i) * 1e9f) / sample_rate));
                size_t offset        = static_cast<size_t>(i) * vector_size;
                bufferData.chunk.assign(in + offset + (vector_size / 2), in + offset + vector_size);
                signalData.ringBuffer->push(bufferData);
            }
        }
    }

private:
    bool handleGetRequest(const FreqDomainContext &requestContext, AcquisitionSpectra &out) {
        std::string requestedSignal = requestContext.channelNameFilter;
        if (!_signalsMap.contains(requestedSignal)) {
            return false;
        }

        bool result = pollSignal(requestedSignal, requestContext.lastRefTrigger, out);
        return result;
    }

    bool pollSignal(const std::string &requestedSignal, int64_t lastRefTrigger, AcquisitionSpectra &out) {
        auto signalData     = _signalsMap.at(requestedSignal);

        out.refTriggerStamp = 0;
        out.channelName     = requestedSignal;

        std::vector<RingBufferData> currentValues;
        signalData.ringBuffer->get_all(currentValues);
        size_t             chunkSize = 0;
        int                numData   = 0;
        std::vector<float> stridedValues;
        if (!currentValues.empty()) {
            out.channelMagnitude_values.clear(); // TODO is clear working?
            out.channelMagnitude_dim1_discrete_time_values.clear();
            out.channelTimeSinceRefTrigger.clear();
            int64_t firstTimestamp = 0;
            for (RingBufferData bufferData : currentValues) {
                if (bufferData.timestamp > lastRefTrigger) {
                    if (firstTimestamp == 0) {
                        firstTimestamp      = bufferData.timestamp;
                        out.refTriggerStamp = bufferData.timestamp;
                    }
                    chunkSize = bufferData.chunk.size();
                    stridedValues.insert(stridedValues.end(), bufferData.chunk.begin(), bufferData.chunk.end());
                    out.channelMagnitude_dim1_discrete_time_values.push_back(bufferData.timestamp);
                    out.channelTimeSinceRefTrigger.push_back(static_cast<float>(bufferData.timestamp - firstTimestamp) / 1e9f);
                    numData++;
                }
            }
            out.channelMagnitude_values = opencmw::MultiArray<float, 2>(std::move(stridedValues), { static_cast<uint32_t>(numData), static_cast<uint32_t>(chunkSize) });
            //  generate frequency values
            const int   vectorSize = static_cast<int>(chunkSize);
            const float sampleRate = signalData.sink->get_sample_rate();
            out.channelMagnitude_dim2_discrete_freq_values.clear();
            out.channelMagnitude_dim2_discrete_freq_values.reserve(chunkSize);
            const float freqStartValue = 0;
            const float freqStepValue  = 0.5f * sampleRate / static_cast<float>(vectorSize);
            for (int i = 0; i < vectorSize; i++) {
                out.channelMagnitude_dim2_discrete_freq_values.push_back(freqStartValue + static_cast<float>(i) * freqStepValue);
            }
            return true;
        }
        return false;
    }
};

#endif /* FREQUENCY_DOMAIN_WORKER_H */
