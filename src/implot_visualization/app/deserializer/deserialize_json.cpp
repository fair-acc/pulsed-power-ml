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

    // Destride array
    for (int i = 0; i < signalNames.size(); i++) {
        this->buffers[i].signalName = this->signalNames[i];
        int stride                  = this->strideArray.dims[1];
        int offset                  = i * stride;

        for (int j = 0; j < stride; j++) {
            absoluteTimestamp = this->refTrigger_s + this->relativeTimestamps[j];
            value             = this->strideArray.values[offset + j];
            this->buffers[i].addPoint(absoluteTimestamp, value);
        }
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

    this->lastRefTrigger = this->refTrigger_ns;
    addToBuffers();
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

    this->lastRefTrigger = this->refTrigger_ns;
    addToBuffers();
}