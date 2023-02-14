#ifndef FREQUENCY_DOMAIN_WORKER_H
#define FREQUENCY_DOMAIN_WORKER_H

#include "Ringbuffer.hpp"
//#include <disruptor/RingBuffer.hpp>
#include <majordomo/Worker.hpp>

#include <gnuradio/pulsed_power/opencmw_freq_sink.h>

#include <chrono>
#include <unordered_map>

using opencmw::Annotated;
using opencmw::NoUnit;

struct FreqDomainContext {
    std::string             channelNameFilter;
    int32_t                 acquisitionModeFilter = 0; // STREAMING
    std::string             triggerNameFilter;
    int32_t                 maxClientUpdateFrequencyFilter = 25;
    opencmw::MIME::MimeType contentType                    = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(FreqDomainContext, channelNameFilter, acquisitionModeFilter, triggerNameFilter, maxClientUpdateFrequencyFilter, contentType)

struct AcquisitionSpectra {
    std::string        refTriggerName  = { "NO_REF_TRIGGER" };
    int64_t            refTriggerStamp = 0;
    std::string        channelName;
    std::vector<float> channelMagnitudeValues;
    std::vector<float> channelFrequencyValues;
    std::string        channelUnit;
};

ENABLE_REFLECTION_FOR(AcquisitionSpectra, refTriggerName, refTriggerStamp, channelName, channelMagnitudeValues, channelFrequencyValues, channelUnit)

using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class FrequencyDomainWorker
    : public Worker<serviceName, FreqDomainContext, Empty, AcquisitionSpectra, Meta...> {
private:
    static const size_t RING_BUFFER_SIZE = 128;
    const std::string   _deviceName;
    std::atomic<bool>   _shutdownRequested;
    std::jthread        _pollingThread;
    AcquisitionSpectra  _reply;

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
    using super_t = Worker<serviceName, FreqDomainContext, Empty, AcquisitionSpectra, Meta...>;

    template<typename BrokerType>
    explicit FrequencyDomainWorker(const BrokerType &broker)
        : super_t(broker, {}) {
        // polling thread
        _pollingThread = std::jthread([this] {
            std::chrono::duration<double, std::milli> pollingDuration;
            while (!_shutdownRequested) {
                std::chrono::time_point time_start = std::chrono::system_clock::now();

                for (auto subTopic : super_t::activeSubscriptions()) { // loop over active subscriptions
                    if (subTopic.path() != "/AcquisitionSpectra") {
                        break;
                    }
                    const auto                         queryMap = subTopic.queryParamMap();
                    const FreqDomainContext            filterIn = opencmw::query::deserialise<FreqDomainContext>(queryMap);
                    std::set<std::string, std::less<>> requestedSignals;
                    if (!checkRequestedSignals(filterIn, requestedSignals)) {
                        break;
                    }

                    pollSignal(*(requestedSignals.begin()), _reply);
                    FreqDomainContext filterOut = filterIn;
                    filterOut.contentType       = opencmw::MIME::JSON;
                    super_t::notify("/AcquisitionSpectra", filterOut, _reply);
                }
                pollingDuration   = std::chrono::system_clock::now() - time_start;

                auto willSleepFor = std::chrono::milliseconds(40) - pollingDuration;
                std::this_thread::sleep_for(willSleepFor);
            }
        });

        // map signal names and ringbuffers, register callback
        std::scoped_lock lock(gr::pulsed_power::globalFrequencySinksRegistryMutex);
        fmt::print("GR: number of frequency-domain sinks found: {}\n", gr::pulsed_power::globalFrequencySinksRegistry.size());
        for (auto sink : gr::pulsed_power::globalFrequencySinksRegistry) {
            const auto signal_names = sink->get_signal_names();
            const auto signal_units = sink->get_signal_units();
            const auto sample_rate  = sink->get_sample_rate();

            for (size_t i = 0; i < signal_names.size(); i++) {
                // init RingBuffer, register poller and poller sequence
                auto       ringbuffer         = std::make_shared<Ringbuffer<RingBufferData>>(RING_BUFFER_SIZE);
                const auto completeSignalName = fmt::format("{}@{}Hz", signal_names[i], sample_rate);
                _signalsMap.insert({ completeSignalName, SignalData(sink, ringbuffer) });
                fmt::print("GR: OpenCMW Frequency Sink '{}' added\n", completeSignalName);
            }

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

    ~FrequencyDomainWorker() {
        _shutdownRequested = true;
        _pollingThread.join();
    }

    void callbackCopySinkData(std::vector<const void *> &input_items, int &nitems, size_t vector_size, const std::vector<std::string> &signal_name, float sample_rate, int64_t timestamp) {
        const float *in                 = static_cast<const float *>(input_items[0]);
        const auto   completeSignalName = fmt::format("{}@{}Hz", signal_name[0], sample_rate);
        if (_signalsMap.contains(completeSignalName)) {
            const SignalData &signalData = _signalsMap.at(completeSignalName);

            for (int i = 0; i < nitems; i++) {
                // publish data
                RingBufferData bufferData;
                bufferData.timestamp = timestamp;
                size_t offset        = static_cast<size_t>(i) * vector_size;
                bufferData.chunk.assign(in + offset + (vector_size / 2), in + offset + vector_size);
                signalData.ringBuffer->push(bufferData);
            }
        }
    }

private:
    bool handleGetRequest(const FreqDomainContext &requestContext, AcquisitionSpectra &out) {
        std::set<std::string, std::less<>> requestedSignals;
        if (!checkRequestedSignals(requestContext, requestedSignals)) {
            return false;
        }

        bool result = pollSignal(*(requestedSignals.begin()), out);
        return result;
    }

    bool pollSignal(const std::string &requestedSignal, AcquisitionSpectra &out) {
        auto signalData     = _signalsMap.at(requestedSignal);

        out.refTriggerStamp = 0;
        out.channelName     = requestedSignal;

        std::vector<RingBufferData> new_values;
        signalData.ringBuffer->get_all(new_values);
        if (!new_values.empty()) {
            RingBufferData last_val = new_values.back();
            out.refTriggerStamp = last_val.timestamp;
            out.channelMagnitudeValues.assign(last_val.chunk.begin(), last_val.chunk.end());

            //  generate frequency values
            size_t vectorSize = out.channelMagnitudeValues.size();
            float  bandwidth  = signalData.sink->get_bandwidth();
            out.channelFrequencyValues.clear();
            out.channelFrequencyValues.reserve(vectorSize);
            float freqStartValue = 0; //-(bandwidth / 2);
            float freqStepValue  = 0.5f * bandwidth / static_cast<float>(vectorSize);
            for (size_t i = 0; i < vectorSize; i++) {
                out.channelFrequencyValues.push_back(freqStartValue + static_cast<float>(i) * freqStepValue);
            }
            return true;
        }
        return false;
    }

    bool checkRequestedSignals(const FreqDomainContext &filterIn, std::set<std::string, std::less<>> &requestedSignals) {
        auto signals = std::string_view(filterIn.channelNameFilter) | std::ranges::views::split(',');
        for (const auto &signal : signals) {
            requestedSignals.emplace(std::string_view(signal.begin(), signal.end()));
        }
        if (requestedSignals.empty()) {
            respondWithEmptyResponse(filterIn, "no signals requested, sending empty response\n");
            return false;
        }

        // check if signals exist
        std::vector<std::string> unknownSignals;
        for (const auto &requestedSignal : requestedSignals) {
            if (!_signalsMap.contains(requestedSignal)) {
                unknownSignals.push_back(requestedSignal);
            }
        }
        if (!unknownSignals.empty()) {
            respondWithEmptyResponse(filterIn, fmt::format("requested unknown signals: {}\n", unknownSignals));
            return false;
        }

        return true;
    }

    void respondWithEmptyResponse(const FreqDomainContext &filter, const std::string_view errorText) {
        fmt::print("{}\n", errorText);
        super_t::notify("/AcquisitionSpectra", filter, AcquisitionSpectra());
    }
};

#endif /* FREQUENCY_DOMAIN_WORKER_H */
