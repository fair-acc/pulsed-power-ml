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

template<typename T>
IAcquisition<T>::IAcquisition() {}

template<typename T>
IAcquisition<T>::IAcquisition(const std::vector<std::string> _signalNames)
    : signalNames(_signalNames) {
    int            numSignals = _signalNames.size();
    std::vector<T> _buffers(numSignals);
    this->buffers = _buffers;
}

template<typename T>
bool IAcquisition<T>::receivedRequestedSignals(std::vector<std::string> receivedSignals) {
    std::vector<std::string> expectedSignals = this->signalNames;
    if (receivedSignals.size() != expectedSignals.size()) {
        return false;
    }
    for (int i = 0; i < receivedSignals.size(); i++) {
        if (receivedSignals[i] != expectedSignals[i]) {
            return false;
        }
    }
    return true;
}

Acquisition::Acquisition() {}

Acquisition::Acquisition(const std::vector<std::string> &_signalNames)
    : IAcquisition(_signalNames) {}

void Acquisition::addToBuffers() {
    double absoluteTimestamp = 0.0;
    double value             = 0.0;
    int    stride            = this->strideArray.dims[1];

    // Destride array
    for (int i = 0; i < strideArray.dims[0]; i++) {
        this->buffers[i].signalName = this->signalNames[i];
        int offset2                 = i * stride;

        if (strideArray.values.size() != (strideArray.dims[0] * strideArray.dims[1])) {
        }
        for (int j = 0; j < stride; j++) {
            absoluteTimestamp = this->refTrigger_s + this->relativeTimestamps[j];
            value             = this->strideArray.values[offset2 + j];
            this->buffers[i].addPoint(absoluteTimestamp, value);
        }
    }
}

void Acquisition::deserialize() {
    auto json_obj = json::parse(jsonString);
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
            if (!this->receivedRequestedSignals(element.value())) {
                std::cout << "Received other signals than requested" << std::endl;
                return;
            }
        } else if (element.key() == "channelValues") {
            this->strideArray.dims   = std::vector<int>(element.value()["dims"]);
            this->strideArray.values = std::vector<double>(element.value()["values"]);
        }
    }

    this->lastRefTrigger = this->refTrigger_ns;
    this->lastTimeStamp  = this->lastRefTrigger + relativeTimestamps.back() * 1e9;
    addToBuffers();
}

AcquisitionSpectra::AcquisitionSpectra() {}

AcquisitionSpectra::AcquisitionSpectra(const std::vector<std::string> &_signalNames)
    : IAcquisition(_signalNames) {
}

void AcquisitionSpectra::addToBuffers() {
    for (int i = 0; i < signalNames.size(); i++) {
        this->buffers[i].signalName = this->signalNames[i];
        this->buffers[i].assign(this->channelFrequencyValues, this->channelMagnitudeValues);
    }
}

void AcquisitionSpectra::deserialize() {
    auto json_obj = json::parse(jsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = this->refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelName") {
            if (this->receivedRequestedSignals(element.value())) {
                std::cout << "Received other signals than requested" << std::endl;
                return;
            }
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

template class IAcquisition<Buffer>;
template class IAcquisition<ScrollingBuffer>;
