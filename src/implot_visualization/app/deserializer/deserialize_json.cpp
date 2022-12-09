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
    if (!this->data.empty()) {
        this->data.shrink(0);
    }
    for (int i = 0; i < x.size(); i++) {
        this->data.push_back(DataPoint(x[i], y[i]));
    }
}

ScrollingBuffer::ScrollingBuffer(int max_size) {
    this->maxSize = max_size;
    this->offset  = 0;
    this->x_data.reserve(this->maxSize);
    this->y_data.reserve(this->maxSize);
}

void ScrollingBuffer::addPoint(double x, double y) {
    if (this->x_data.size() < this->maxSize){
        this->x_data.push_back(x);
        this->y_data.push_back(y);
    } else {
        this->x_data[this->offset] = x;
        this->y_data[this->offset] = y;
        this->offset               = (this->offset + 1) % this->maxSize;
    }
}

void ScrollingBuffer::addPoints(std::span<double> x, std::span<double> y) {
    if (this->x_data.size() < this->maxSize) {
        const auto newSize = std::min(maxSize, x_data.size() + x.size());
        this->x_data.resize(newSize);
        this->y_data.resize(newSize);
    }
    auto start = x.begin();
    auto end = std::min(x.end(), x.begin() + maxSize - offset);
    auto y_start = y.begin();
    auto y_end = std::min(y.end(), y.begin() + maxSize - offset);
    std::copy(start, end, x_data.begin() + offset);
    std::copy(y_start, y_end, y_data.begin() + offset);
    offset += end - start;
    if (end == x.end()) return;
    start = end;
    end = x.end();
    y_start = y_end;
    y_end = y.end();
    std::copy(start, end, x_data.begin());
    std::copy(y_start, y_end, y_data.begin());
    offset = start - end;
}

void ScrollingBuffer::erase() {
    if (!this->x_data.empty()) {
        this->x_data.resize(0);
        this->y_data.resize(0);
        this->offset = 0;
    }
}

Acquisition::Acquisition() {}

Acquisition::Acquisition(int _numSignals) {
    std::vector<ScrollingBuffer> _buffers(_numSignals);
    this->buffers = _buffers;
}

void Acquisition::addToBuffers(std::vector<int> dims, std::vector<double> values, std::vector<double> relativeTimestamps) {
    double absoluteTimestamp = 0.0;
    double value             = 0.0;

    double refTrigger_s = this->refTrigger_ns * 1e-9;
    // Destride array
    for (int i = 0; i < signalNames.size(); i++) {
        this->buffers[i].signalName = this->signalNames[i];
        int stride                  = dims[1];
        int offset                  = i * stride;
        std::vector<double> timescale = std::vector<double>(stride);

        for (int j = 0; j < stride; j++) {
            absoluteTimestamp = refTrigger_s + relativeTimestamps[j];
            value             = values[offset + j];
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
    std::vector<int> dims;
    std::vector<double> values;
    std::vector<double> relativeTimestamps;
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            if (element.value() == 0) {
                return;
            }
            this->refTrigger_ns = element.value();
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            relativeTimestamps.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelNames") {
            this->signalNames.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelValues") {
            dims   = std::vector<int>(element.value()["dims"]);
            values = std::vector<double>(element.value()["values"]);
        }
    }

    this->lastRefTrigger = this->refTrigger_ns;
    this->lastTimeStamp  = this->lastRefTrigger + relativeTimestamps.back() * 1e9;
    // Debug
    std::cout << "lastRefTrigger: " << lastRefTrigger << std::endl;
    std::cout << "lastTimeStamp: " << lastTimeStamp << std::endl;
    values[1] = -6;
    addToBuffers(dims, values, relativeTimestamps);
}

AcquisitionSpectra::AcquisitionSpectra() {}

AcquisitionSpectra::AcquisitionSpectra(int _numSignals) {
    std::vector<Buffer> _buffers(_numSignals);
    this->buffers = _buffers;
}

void AcquisitionSpectra::addToBuffers(std::vector<double> channelMagnitudeValues, std::vector<double> channelFrequencyValues) {
    this->buffers[0].signalName = this->signalName;
    this->buffers[0].assign(channelFrequencyValues, channelMagnitudeValues);
}

void AcquisitionSpectra::deserialize() {
    if (this->jsonString.substr(0, 21) != "\"AcquisitionSpectra\":") {
        return;
    }
    std::string modifiedJsonString = this->jsonString;

    modifiedJsonString.erase(0, 21);

    std::vector<double> channelMagnitudeValues;
    std::vector<double> channelFrequencyValues;
    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            this->refTrigger_ns = element.value();
        } else if (element.key() == "channelName") {
            this->signalName = element.value();
        } else if (element.key() == "channelMagnitudeValues") {
            channelMagnitudeValues.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelFrequencyValues") {
            channelFrequencyValues.assign(element.value().begin(), element.value().end());
        }
    }
    lastTimeStamp        = lastRefTrigger;
    this->lastRefTrigger = this->refTrigger_ns;
    addToBuffers(channelMagnitudeValues, channelFrequencyValues);
}
