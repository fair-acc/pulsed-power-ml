#include <cmath>
#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

constexpr DataPoint::DataPoint()
    : x(0.0f), y(0.0f) {}
constexpr DataPoint::DataPoint(double _x, double _y)
    : x(_x), y(_y) {}

Buffer::Buffer(int max_size) {
    this->data.reserve(max_size);
    this->data.reserve(max_size);
}

void Buffer::assign(const std::vector<double> &x, const std::vector<double> &y) {
    if (this->data.size() > 0) {
        this->data.shrink(0);
    }
    for (int i = 0; i < x.size(); i++) {
        this->data.push_back(DataPoint(x[i], y[i]));
    }
}

ScrollingBuffer::ScrollingBuffer(int max_size) {
    this->maxSize = max_size;
    this->offset  = 0;
    this->data.reserve(this->maxSize);
}

void ScrollingBuffer::addPoint(double x, double y) {
    if (this->data.size() < this->maxSize)
        this->data.push_back(DataPoint(x, y));
    else {
        this->data[this->offset] = DataPoint(x, y);
        this->offset             = (this->offset + 1) % this->maxSize;
    }
}

void ScrollingBuffer::erase() {
    if (this->data.size() > 0) {
        this->data.shrink(0);
        this->offset = 0;
    }
}

Acquisition::Acquisition() {}

Acquisition::Acquisition(int _numSignals) {
    std::vector<ScrollingBuffer> _buffers(_numSignals);
    this->buffers = _buffers;
}

void Acquisition::addToBuffers() {
    double absoluteTimestamp = 0.0;
    double value             = 0.0;
    int    stride            = this->strideArray.dims[1];

    // Destride array
    for (int i = 0; i < strideArray.dims[0]; i++) {
        this->buffers[i].signalName = this->signalNames[i];
        int offset2                 = i * stride;

        std::cout << "DestrideArray start" << std::endl; // debug
        if (strideArray.values.size() != (strideArray.dims[0] * strideArray.dims[1])) {
            std::cout << "So eine Kacke!" << std::endl; // debug
        }
        // std::cout << "Array dims: [" << strideArray.dims[0] << ", " << strideArray.dims[1] << "], "; // debug
        // std::cout << "Array length: " << strideArray.values.size() << std::endl;                     // debug
        for (int j = 0; j < stride; j++) {
            // std::cout << "Calculate timestamp" << std::endl;
            // std::cout << "j: " << j << ", relativeTimestamps length: " << relativeTimestamps.size() << std::endl; // debug
            absoluteTimestamp = this->refTrigger_s + this->relativeTimestamps[j];
            // std::cout << "Calculate value" << std::endl;
            // std::cout << "OffsetIndex: " << (offset + j) << ", Array length: " << strideArray.values.size() << std::endl; // debug
            value = this->strideArray.values[offset2 + j];
            // std::cout << "addPoint()" << std::endl;
            this->buffers[i].addPoint(absoluteTimestamp, value);
        }
        std::cout << "DestrideArray finished" << std::endl; // debug
    }
}

void Acquisition::deserialize() {
    if (this->jsonString.substr(0, 14) != "\"Acquisition\":") {
        return;
    }
    std::string modifiedJsonString = this->jsonString;

    modifiedJsonString.erase(0, 14);

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            if (element.value() == 0) {
                return;
            }
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            this->relativeTimestamps.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelNames") {
            this->signalNames.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelValues") {
            this->strideArray.dims   = std::vector<int>(element.value()["dims"]);
            this->strideArray.values = std::vector<double>(element.value()["values"]);
        }
    }
    std::cout << "SetLastTImestamp start" << std::endl; // debug

    this->lastRefTrigger = this->refTrigger_ns;
    this->lastTimeStamp  = this->lastRefTrigger + relativeTimestamps.back() * 1e9;
    std::cout << "SetLastTImestamp finished" << std::endl; // debug

    std::cout << "AddToBuffers start" << std::endl; // debug

    addToBuffers();
    std::cout << "AddToBuffers finished" << std::endl; // debug
}

AcquisitionSpectra::AcquisitionSpectra() {}

AcquisitionSpectra::AcquisitionSpectra(int _numSignals) {
    std::vector<Buffer> _buffers(_numSignals);
    this->buffers = _buffers;
}

void AcquisitionSpectra::addToBuffers() {
    this->buffers[0].signalName = this->signalName;
    this->buffers[0].assign(this->channelFrequencyValues, this->channelMagnitudeValues);
}

void AcquisitionSpectra::deserialize() {
    if (this->jsonString.substr(0, 21) != "\"AcquisitionSpectra\":") {
        return;
    }
    std::string modifiedJsonString = this->jsonString;

    modifiedJsonString.erase(0, 21);

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = this->refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelName") {
            this->signalName = element.value();
        } else if (element.key() == "channelMagnitudeValues") {
            this->channelMagnitudeValues.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelFrequencyValues") {
            this->channelFrequencyValues.assign(element.value().begin(), element.value().end());
        }
    }
    lastTimeStamp        = lastRefTrigger;
    this->lastRefTrigger = this->refTrigger_ns;
    addToBuffers();
}
