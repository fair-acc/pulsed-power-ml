#include <cmath>
#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json        = nlohmann::json;

double tmp_t_prev = 0.0;

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

double absoluteTimestampPrev = 0.0;

bool   timestampsInBufferAreEquidistant(SignalBuffer &buffer) {
    bool   isEquidistant = true;
    double timestampCurr = buffer.data[0].x;
    double timestampPrev = buffer.data[0].x;
    double dt            = 0.0;
    for (int i = 0; i < buffer.data.size(); i++) {
        timestampCurr = buffer.data[i].x;
        dt            = timestampCurr - timestampPrev;
        // std::cout << "dt: " << dt << std::endl;
        if (dt < 0 || dt > 0.001) {
            isEquidistant = false;
            std::cout << "Timestamps in Buffer are not equidistant!" << std::endl;
            // std::cout << "Previous timestamp: " << timestampPrev << std::endl;
            // std::cout << "Current timestamp: " << timestampCurr << std::endl;
            std::cout << "dt: " << dt << std::endl;
            std::cout << "Buffer index: " << i << std::endl;
        }
        timestampPrev = timestampCurr;
    }
    return isEquidistant;
}

void Deserializer::addToSignalBuffers(std::vector<SignalBuffer> &signals, const Acquisition &acquisitionData) {
    double absoluteTimestamp = acquisitionData.refTrigger_s;
    double value             = 0.0;

    for (int i = 0; i < acquisitionData.signalNames.size(); i++) {
        signals[i].addFinished = false;
        signals[i].signalName  = acquisitionData.signalNames[i];
        int stride             = acquisitionData.strideArray.dims[1];
        int offset             = i * stride;

        for (int j = 0; j < stride; j++) {
            absoluteTimestampPrev = absoluteTimestamp;
            absoluteTimestamp     = acquisitionData.refTrigger_s + acquisitionData.relativeTimestamps[j];
            value                 = acquisitionData.strideArray.values[offset + j];
            signals[i].addPoint(absoluteTimestamp, value);

            // debug
            // if (acquisitionData.signalNames[i] == "square@4000Hz" && value < 0) {
            //     std::cout << "Negative value detected!" << std::endl;
            //     std::cout << "Json: " << value << std::endl;
            //     // std::cout << "StrideArray dims: " << acquisitionData.strideArray.dims[0] << ", " << acquisitionData.strideArray.dims[1] << std::endl;
            //     // std::cout << "Index: " << offset + j << std::endl;
            //     // std::cout << "Array length: " << acquisitionData.strideArray.values.size() << std::endl;
            //     // std::cout << "Buffer: " << signals[i].back().y << std::endl;
            // }
            // if (absoluteTimestamp - absoluteTimestampPrev > 0.005) {
            //     std::cout << "Data loss detected by missing timestamps!" << std::endl;
            //     std::cout << "Previous timestamp: " << absoluteTimestampPrev << std::endl;
            //     std::cout << "Current timestamp: " << absoluteTimestamp << std::endl;
            // }
        }

        signals[i].addFinished = true;
    }
}

void Deserializer::deserializeAcquisition(const std::string &jsonString, std::vector<SignalBuffer> &signals) {
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
            // std::cout << "StrideArray dims: " << acquisition.strideArray.dims[0] << ", " << acquisition.strideArray.dims[1] << std::endl;
            // std::cout << "Array length: " << acquisition.strideArray.values.size() << std::endl;
            // if (acquisition.strideArray.dims[0] * acquisition.strideArray.dims[1] != acquisition.strideArray.values.size()) {
            //     std::cout << "Vector size does not match dimensions!" << std::endl;
            // }
        }
    }

    addToSignalBuffers(signals, acquisition);
    timestampsInBufferAreEquidistant(signals[0]);
}

void Deserializer::deserializeCounter(const std::string &jsonString, std::vector<SignalBuffer> &signals) {
    double      timestamp          = 0.0;
    double      value              = 0.0;

    std::string modifiedJsonString = jsonString;
    modifiedJsonString.erase(0, 14);

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "timestamp") {
            timestamp = element.value();
        } else {
            value = element.value();
        }
    }

    if (timestamp != tmp_t_prev) {
        signals[0].addPoint(timestamp, value);
    }

    tmp_t_prev = timestamp;
}

void Deserializer::deserializeJson(const std::string &jsonString, std::vector<SignalBuffer> &signals) {
    // std::cout << jsonString << std::endl;
    if (jsonString.substr(1, 11) == "Acquisition") {
        deserializeAcquisition(jsonString, signals);
    } else if (jsonString.substr(1, 11) == "CounterData") {
        deserializeCounter(jsonString, signals);
    }
}