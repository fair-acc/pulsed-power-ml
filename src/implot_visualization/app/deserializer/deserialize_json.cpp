#include "deserialize_json.h"
#include <cmath>
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

void PowerBuffer::updateValues(const std::vector<double> &_values) {
    this->values       = _values;
    auto   clock       = std::chrono::system_clock::now();

    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    this->timestamp    = currentTime;
    init               = true;
}

template<typename T>
IAcquisition<T>::IAcquisition() {}

template<typename T>
IAcquisition<T>::IAcquisition(const std::vector<std::string> _signalNames, const int _bufferSize)
    : signalNames(_signalNames) {
    T buffer(_bufferSize);
    for (auto name : _signalNames) {
        this->buffers.push_back(buffer);
    }
}

template<typename T>
bool IAcquisition<T>::receivedRequestedSignals(std::vector<std::string> receivedSignals) {
    std::vector<std::string> expectedSignals = this->signalNames;
    if (receivedSignals.size() != expectedSignals.size()) {
        std::cout << "received size: " << receivedSignals.size() << ", expected size: " << expectedSignals.size() << std::endl;
        return false;
    }
    for (int i = 0; i < receivedSignals.size(); i++) {
        if (receivedSignals[i] != expectedSignals[i]) {
            std::cout << "received: " << receivedSignals[i] << ", expected: " << expectedSignals[i] << std::endl;
            return false;
        }
    }
    return true;
}

StrideArray::StrideArray() {
    this->dims   = { 0, 0 };
    this->values = {};
}

void StrideArray::cut(const std::vector<int> newDims) {
    StrideArray cutStrideArray;
    cutStrideArray.dims = newDims;
    int    stride       = this->dims[1];
    int    newStride    = newDims[1];
    double value        = 0.0;
    for (int i = 0; i < this->dims[0]; i++) {
        int offset = i * this->dims[1];

        if (this->values.size() != (this->dims[0] * this->dims[1])) {
            std::cout << "Invalid dimensions for strided array." << std::endl;
        }
        for (int j = 0; j < stride; j++) {
            value = this->values[offset + j];
            cutStrideArray.values.push_back(value);
            // Cut stride
            if (j >= newStride - 1) {
                break;
            }
        }
    }
    this->dims   = cutStrideArray.dims;
    this->values = cutStrideArray.values;
}

bool Acquisition::receivedVoltageCurrentData(std::vector<std::string> receivedSignals) {
    bool containsCurrent = false;
    bool containsVoltage = false;
    for (auto signalName : receivedSignals) {
        if (signalName.find("I@") != std::string::npos)
            containsCurrent = true;
        if (signalName.find("U@") != std::string::npos)
            containsVoltage = true;
    }
    return containsCurrent * containsVoltage;
}

const ConvertPair convertValues(const StrideArray &strideArray, const std::vector<double> &relativeTimestamps) {
    ConvertPair         ret;
    StrideArray         newStrideArray = strideArray;
    std::vector<double> newRelativeTimestamps;
    double              newRelativeTimestamp = 60;

    if (relativeTimestamps.size() < 2) {
        std::cout << "Not enough values to calculate relative data for U and I." << std::endl;
        return ret;
    }

    double max   = relativeTimestamps[relativeTimestamps.size() - 1];
    double delta = max - relativeTimestamps[relativeTimestamps.size() - 2];
    delta *= 1000;
    for (auto it = relativeTimestamps.end(); it != relativeTimestamps.begin(); it--) {
        // Calculate new relative timestamps
        newRelativeTimestamps.push_back(newRelativeTimestamp);
        newRelativeTimestamp -= delta;
        if (newRelativeTimestamp < 0) {
            newStrideArray.cut({ strideArray.dims[0], static_cast<int>(newRelativeTimestamps.size()) });
            break;
        }
    }
    std::reverse(newRelativeTimestamps.begin(), newRelativeTimestamps.end());
    ret.relativeTimestamps  = newRelativeTimestamps;
    ret.referenceTimestamps = 0;
    ret.strideArray         = newStrideArray;
    return ret;
}

Acquisition::Acquisition() {}

Acquisition::Acquisition(const std::vector<std::string> &_signalNames, const int _bufferSize)
    : IAcquisition(_signalNames, _bufferSize) {}

void Acquisition::addToBuffers(const StrideArray &strideArray, const std::vector<double> &relativeTimestamps, double refTrigger_ns) {
    double absoluteTimestamp = 0.0;
    double value             = 0.0;
    int    stride            = strideArray.dims[1];
    double refTrigger_s      = refTrigger_ns / std::pow(10, 9);

    // Destride array
    for (int i = 0; i < strideArray.dims[0]; i++) {
        this->buffers[i].signalName = this->signalNames[i];
        int offset                  = i * stride;

        if (strideArray.values.size() != (strideArray.dims[0] * strideArray.dims[1])) {
            std::cout << "Invalid dimensions for strided array." << std::endl;
        }
        for (int j = 0; j < stride; j++) {
            absoluteTimestamp = refTrigger_s + relativeTimestamps[j];
            value             = strideArray.values[offset + j];
            this->buffers[i].addPoint(absoluteTimestamp, value);
        }
    }
}

void Acquisition::deserialize() {
    std::vector<double> relativeTimestamps = {};
    uint64_t            refTrigger_ns      = 0;
    StrideArray         strideArray;
    bool                convertValuesBool = false;
    if (!json::accept(this->jsonString)) {
        std::cout << "Invalid json string in Aquisition: " << this->jsonString << std::endl;
        return;
    }
    auto json_obj = json::parse(this->jsonString);
    // empty response
    if (json_obj["refTriggerStamp"] == 0) {
        return;
    }
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            refTrigger_ns = element.value();
        } else if (element.key() == "channelNames") {
            if (!this->receivedRequestedSignals(element.value())) {
                std::cout << "Received other signals than requested (Acquisition)" << std::endl;
                return;
            }
            std::cout << "Received expected signal (Acquisition)" << std::endl;
            convertValuesBool = receivedVoltageCurrentData(element.value());
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            relativeTimestamps.assign(element.value().begin(), element.value().end());

        } else if (element.key() == "channelValues") {
            strideArray.dims   = std::vector<int>(element.value()["dims"]);
            strideArray.values = std::vector<double>(element.value()["values"]);
        }
    }
    this->lastRefTrigger = refTrigger_ns;
    if (!relativeTimestamps.empty()) {
        this->lastTimeStamp = this->lastRefTrigger + relativeTimestamps.back() * 1e9;
        if (convertValuesBool) {
            ConvertPair ret    = convertValues(strideArray, relativeTimestamps);
            relativeTimestamps = ret.relativeTimestamps;
            refTrigger_ns      = ret.referenceTimestamps;
            strideArray        = ret.strideArray;
        }
        addToBuffers(strideArray, relativeTimestamps, refTrigger_ns);
    } else {
        this->lastTimeStamp = this->lastRefTrigger;
    }
}

AcquisitionSpectra::AcquisitionSpectra() {}

AcquisitionSpectra::AcquisitionSpectra(const std::vector<std::string> &_signalNames, const int _bufferSize)
    : IAcquisition(_signalNames, _bufferSize) {}

void AcquisitionSpectra::addToBuffers(const std::vector<double> &channelFrequencyValues, const std::vector<double> &channelMagnitudeValues) {
    for (int i = 0; i < signalNames.size(); i++) {
        this->buffers[i].signalName = this->signalNames[i];
        this->buffers[i].assign(channelFrequencyValues, channelMagnitudeValues);
    }
}

void AcquisitionSpectra::deserialize() {
    std::vector<double> channelMagnitudeValues;
    std::vector<double> channelFrequencyValues;
    uint64_t            refTrigger_ns      = 0;
    std::vector<double> relativeTimestamps = {};
    if (!json::accept(this->jsonString)) {
        std::cout << "Invalid json string in AquisitionSpectra: " << this->jsonString << std::endl;
        return;
    }
    auto json_obj = json::parse(this->jsonString);
    // empty response
    if (json_obj["refTriggerStamp"] == 0) {
        return;
    }
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            refTrigger_ns = element.value();
        } else if (element.key() == "channelName") {
            std::vector<std::string> channelNames = { element.value().get<std::string>() };
            if (!this->receivedRequestedSignals(channelNames)) {
                std::cout << "Received other signals than requested (AcquisitionSpectra)" << std::endl;
                return;
            }
            std::cout << "Received expected signal (AcquisitionSpectra)" << std::endl;
        } else if (element.key() == "channelMagnitude_values") {
            int vectorSize = element.value()["dims"][1];
            int numValues  = element.value()["dims"][0];
            channelMagnitudeValues.clear(); // ??? Can be removed ???
            for (int i = vectorSize * (numValues - 1); i < vectorSize * numValues; i++) {
                channelMagnitudeValues.push_back(element.value()["values"][i]);
            }
        } else if (element.key() == "channelMagnitude_dim2_discrete_freq_values") {
            channelFrequencyValues.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            relativeTimestamps.assign(element.value().begin(), element.value().end());
        }
    }
    if (!relativeTimestamps.empty()) {
        this->lastTimeStamp = refTrigger_ns + relativeTimestamps.back() * 1e9;
    } else {
        this->lastTimeStamp = refTrigger_ns;
    }
    addToBuffers(channelFrequencyValues, channelMagnitudeValues);
}

PowerUsage::PowerUsage() {}

PowerUsage::PowerUsage(const std::vector<std::string> &_signalNames, const int _bufferSize)
    : IAcquisition(_signalNames, _bufferSize) {}

void PowerUsage::deserialize() {
    if (!json::accept(this->jsonString)) {
        std::cout << "Invalid json string in PowerUsage: " << this->jsonString << std::endl;
        return;
    }
    auto json_obj = json::parse(this->jsonString);
    // empty response
    if (json_obj["timestamp"] == 0) {
        return;
    }
    for (auto &element : json_obj.items()) {
        if (element.key() == "values") {
            this->powerUsages.clear();
            this->powerUsages.assign(element.value().begin(), element.value().end());
        }
        if (element.key() == "names") {
            this->devices.clear();
            this->devices.assign(element.value().begin(), element.value().end());
        }
        if (element.key() == "timestamp") {
            timestamp = element.value();
        }
        if (element.key() == "dayUsage") {
            if (element.value().size() == 0) {
                return;
            }
            this->powerUsagesDay.clear();
            this->powerUsagesDay.assign(element.value().begin(), element.value().end());
        }
        if (element.key() == "weekUsage") {
            if (element.value().size() == 0) {
                return;
            }
            this->powerUsagesWeek.clear();
            this->powerUsagesWeek.assign(element.value().begin(), element.value().end());
        }
        if (element.key() == "monthUsage") {
            if (element.value().size() == 0) {
                return;
            }
            this->powerUsagesMonth.clear();
            this->powerUsagesMonth.assign(element.value().begin(), element.value().end());
        }
    }

    this->setSumOfUsageDay();
    this->setSumOfUsageWeek();
    this->setSumOfUsageMonth();

    this->success      = true;
    this->init         = true;
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    this->deliveryTime = currentTime;
}

void PowerUsage::fail() {
    this->success = false;
}

double PowerUsage::sumOfUsage() {
    if (this->init) {
        double sum_of_usage = 0.0;
        for (std::vector<double>::iterator it = this->powerUsages.begin(); it != this->powerUsages.end(); ++it) {
            sum_of_usage += *it;
        }
        return sum_of_usage;
    } else {
        return 0.0;
    }
}

void PowerUsage::setSumOfUsageDay() {
    if (this->init) {
        double sum_of_usage = 0.0;
        for (std::vector<double>::iterator it = this->powerUsagesDay.begin(); it != this->powerUsagesDay.end(); ++it) {
            sum_of_usage += *it;
        }
        this->kWhUsedDay = sum_of_usage;
    }
}

void PowerUsage::setSumOfUsageWeek() {
    double sum_of_usage = 0.0;
    if (this->init) {
        for (std::vector<double>::iterator it = this->powerUsagesWeek.begin(); it != this->powerUsagesWeek.end(); ++it) {
            sum_of_usage += *it;
        }
        this->kWhUsedWeek = sum_of_usage;
    }
}

void PowerUsage::setSumOfUsageMonth() {
    double sum_of_usage = 0.0;
    if (this->init) {
        for (std::vector<double>::iterator it = this->powerUsagesMonth.begin(); it != this->powerUsagesMonth.end(); ++it) {
            sum_of_usage += *it;
        }
        this->kWhUsedMonth = sum_of_usage;
    }
}

RealPowerUsage::RealPowerUsage() {}

RealPowerUsage::RealPowerUsage(const std::vector<std::string> &_signalNames, const int _bufferSize)
    : IAcquisition(_signalNames, _bufferSize) {
    std::vector<double> _realPowerUsages(_signalNames.size());
    this->realPowerUsages = _realPowerUsages;
}

void RealPowerUsage::addPowerUsage(const StrideArray &strideArray) {
    double value  = 0.0;
    int    stride = strideArray.dims[1];

    // Destride array
    for (int i = 0; i < strideArray.dims[0]; i++) {
        int offset = i * stride;

        if (strideArray.values.size() != (strideArray.dims[0] * strideArray.dims[1])) {
            std::cout << "Invalid dimensions for strided array." << std::endl;
        }

        value                    = strideArray.values[offset + stride - 1];
        this->realPowerUsages[i] = value / 1000.0;
    }
}

void RealPowerUsage::deserialize() {
    StrideArray         strideArray;
    uint64_t            refTrigger_ns      = 0;
    std::vector<double> relativeTimestamps = {};
    if (!json::accept(this->jsonString)) {
        std::cout << "Invalid json string in RealPowerUasge: " << this->jsonString << std::endl;
        return;
    }
    auto json_obj = json::parse(jsonString);
    // empty response
    if (json_obj["refTriggerStamp"] == 0) {
        return;
    }
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            refTrigger_ns = element.value();
        } else if (element.key() == "channelNames") {
            if (!this->receivedRequestedSignals(element.value())) {
                std::cout << "Received other signals than requested (RealPowerUsage)" << std::endl;
                return;
            }
            std::cout << "Received expected signal (RealPowerUsage)" << std::endl;
        } else if (element.key() == "channelValues") {
            auto values = std::vector<double>(element.value()["values"]);
            if (!values.empty()) {
                strideArray.dims   = std::vector<int>(element.value()["dims"]);
                strideArray.values = std::vector<double>(element.value()["values"]);
                addPowerUsage(strideArray);
            }
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            relativeTimestamps.assign(element.value().begin(), element.value().end());
        }
    }
    if (!relativeTimestamps.empty()) {
        this->lastTimeStamp = refTrigger_ns + relativeTimestamps.back() * 1e9;
    } else {
        this->lastTimeStamp = refTrigger_ns;
    }

    this->init         = true;
    this->success      = true;

    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    this->deliveryTime = currentTime;
}

void RealPowerUsage::fail() {
    this->success = false;
}

template class IAcquisition<Buffer>;
template class IAcquisition<ScrollingBuffer>;
template class IAcquisition<PowerBuffer>;
