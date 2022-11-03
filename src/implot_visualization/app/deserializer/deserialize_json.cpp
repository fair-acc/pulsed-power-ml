#include <chrono>
#include <cmath>
#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

constexpr DataPoint::DataPoint()
    : x(0.0f), y(0.0f) {}
constexpr DataPoint::DataPoint(double _x, double _y)
    : x(_x), y(_y) {}

SignalBuffer::SignalBuffer(int max_size) {
    maxSize = max_size;
    offset  = 0;
    data.reserve(maxSize);
}

void SignalBuffer::addPoint(double x, double y) {
    if (data.size() < maxSize)
        data.push_back(DataPoint(x, y));
    else {
        data[offset] = DataPoint(x, y);
        offset       = (offset + 1) % maxSize;
    }
}

void SignalBuffer::erase() {
    if (data.size() > 0) {
        data.shrink(0);
        offset = 0;
    }
}

DataPoint SignalBuffer::back() {
    return data[maxSize - 1];
}

void Deserializer::addToSignalBuffers(std::vector<SignalBuffer> &signals, const Acquisition &acquisitionData) {
    double absoluteTimestamp = 0.0;
    double value             = 0.0;

    // Destride array
    for (int i = 0; i < acquisitionData.signalNames.size(); i++) {
        signals[i].signalName = acquisitionData.signalNames[i];
        int stride            = acquisitionData.strideArray.dims[1];
        int offset            = i * stride;

        for (int j = 0; j < stride; j++) {
            absoluteTimestamp = acquisitionData.refTrigger_s + acquisitionData.relativeTimestamps[j];
            value             = acquisitionData.strideArray.values[offset + j];
            signals[i].addPoint(absoluteTimestamp, value);
        }
    }
}

void Deserializer::addToSignalBuffers(std::vector<SignalBuffer> &signals, const AcquisitionSpectra &acquisitionData) {
    signals[0].signalName = acquisitionData.signalName;
    for (int i = 0; i < acquisitionData.channelMagnitudeValues.size(); i++) {
        signals[0].addPoint(acquisitionData.channelFrequencyValues[i], acquisitionData.channelMagnitudeValues[i]);
    }
}

void Deserializer::deserializeAcquisition(std::vector<SignalBuffer> &signals) {
    std::string modifiedJsonString = jsonString;
    Acquisition acquisition;

    modifiedJsonString.erase(0, 14);

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            acquisition.refTrigger_ns = element.value();
            acquisition.refTrigger_s  = acquisition.refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            acquisition.relativeTimestamps.insert(acquisition.relativeTimestamps.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelNames") {
            acquisition.signalNames.insert(acquisition.signalNames.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelValues") {
            acquisition.strideArray.dims   = std::vector<int>(element.value()["dims"]);
            acquisition.strideArray.values = std::vector<double>(element.value()["values"]);
        }
    }

    lastRefTrigger = acquisition.refTrigger_ns;
    addToSignalBuffers(signals, acquisition);
}

void Deserializer::deserializeAcquisitionSpectra(std::vector<SignalBuffer> &signals) {
    std::string        modifiedJsonString = jsonString;
    AcquisitionSpectra acquisitionSpectra;

    modifiedJsonString.erase(0, 21);

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            acquisitionSpectra.refTrigger_ns = element.value();
            acquisitionSpectra.refTrigger_s  = acquisitionSpectra.refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelName") {
            acquisitionSpectra.signalName = element.value();
        } else if (element.key() == "channelMagnitudeValues") {
            acquisitionSpectra.channelMagnitudeValues.insert(acquisitionSpectra.channelMagnitudeValues.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelFrequencyValues") {
            acquisitionSpectra.channelFrequencyValues.insert(acquisitionSpectra.channelFrequencyValues.begin(), element.value().begin(), element.value().end());
        }
    }

    lastRefTrigger = acquisitionSpectra.refTrigger_ns;
    addToSignalBuffers(signals, acquisitionSpectra);
}

void Deserializer::deserializeJson(std::vector<SignalBuffer> &signals) {
    if (jsonString.substr(0, 14) == "\"Acquisition\":") {
        deserializeAcquisition(signals);
    } else if (jsonString.substr(0, 21) == "\"AcquisitionSpectra\":") {
        deserializeAcquisitionSpectra(signals);
    }
}