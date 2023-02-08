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

void BufferPower::updateValues(const std::vector<double> &_values) {
    this->values = _values;
    auto clock   = std::chrono::system_clock::now();
    // double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    this->timestamp    = currentTime;
    init               = true;
}

template<typename T>
IAcquisition<T>::IAcquisition() {}

template<typename T>
IAcquisition<T>::IAcquisition(const std::vector<std::string> _signalNames)
    : signalNames(_signalNames) {
    int numSignals = _signalNames.size();
    for (auto name : _signalNames) {
        if (name == "U@1000Hz" || name == "I@1000Hz" || name == "U_bpf@1000Hz" || name == "I_bpf@1000Hz") {
            this->buffers.emplace(this->buffers.end(), 60);
        } else {
            this->buffers.emplace(this->buffers.end());
        }
    }
    // std::vector<T> _buffers(numSignals);
    // this->buffers = _buffers;
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
    auto json_obj = json::parse(this->jsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            if (element.value() == 0) {
                return;
            }
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = this->refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelNames") {
            if (!this->receivedRequestedSignals(element.value())) {
                std::cout << "Received other signals than requested (Acquisition)" << std::endl;
                return;
            }
            std::cout << "Received expected signal (Acquisition)" << std::endl;
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            this->relativeTimestamps.assign(element.value().begin(), element.value().end());
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
    : IAcquisition(_signalNames) {}

void AcquisitionSpectra::addToBuffers() {
    for (int i = 0; i < signalNames.size(); i++) {
        this->buffers[i].signalName = this->signalNames[i];
        this->buffers[i].assign(this->channelFrequencyValues, this->channelMagnitudeValues);
    }
}

void AcquisitionSpectra::deserialize() {
    auto json_obj = json::parse(this->jsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            if (element.value() == 0) {
                return;
            }
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = this->refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelName") {
            std::vector<std::string> channelNames = { element.value().get<std::string>() };
            if (!this->receivedRequestedSignals(channelNames)) {
                std::cout << "Received other signals than requested (AcquisitionSpectra)" << std::endl;
                return;
            }
            std::cout << "Received expected signal (AcquisitionSpectra)" << std::endl;
        } else if (element.key() == "channelMagnitudeValues") {
            this->channelMagnitudeValues.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelFrequencyValues") {
            this->channelFrequencyValues.assign(element.value().begin(), element.value().end());
        }
    }
    this->lastRefTrigger = this->refTrigger_ns;
    this->lastTimeStamp  = this->lastRefTrigger + relativeTimestamps.back() * 1e9;
    addToBuffers();
}

PowerUsage::PowerUsage() {
    printf("PowerUsage created\n");
    printf("Devices: %s\n", devices[0].c_str());
}
PowerUsage::PowerUsage(int _numSignals) {
    std::vector<Buffer> _buffers(_numSignals);
    this->buffers = _buffers;
}

PowerUsage::PowerUsage(const std::vector<std::string> &_signalNames)
    : signalNames(_signalNames) {
    int numSignals = _signalNames.size();
}

void PowerUsage::deserialize() {
    // printf("Deserialize powerUsage\n");

    auto json_obj = json::parse(this->jsonString);
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
        if (element.key() == "day_usage") {
            this->powerUsagesDay.clear();
            this->powerUsagesDay.assign(element.value().begin(), element.value().end());
        }
        if (element.key() == "week_usage") {
            this->powerUsagesWeek.clear();
            this->powerUsagesWeek.assign(element.value().begin(), element.value().end());
        }
        if (element.key() == "month_usage") {
            this->powerUsagesMonth.clear();
            this->powerUsagesMonth.assign(element.value().begin(), element.value().end());
        }
        if (this->powerUsages.size() != this->devices.size()) {
            printf("Number of Values is not equal to number of devices\n");
        }
        //   printf("After deserialization %s\n", this->devices[0].c_str());
        //  printf("usages %d, devices %d\n", this->powerUsages.size(), this->devices.size());
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

RealPowerUsage::RealPowerUsage(){};

RealPowerUsage::RealPowerUsage(int _numSignals) {
}

RealPowerUsage::RealPowerUsage(const std::vector<std::string> &_signalNames)
    : signalNames(_signalNames) {
}

void RealPowerUsage::deserialize() {
    std::string modifiedJsonString = this->jsonString;

    auto        json_obj           = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "channelValues") {
            auto values = std::vector<double>(element.value()["values"]);
            if (!values.empty()) {
                this->realPowerUsageOrig = values.back();

                this->realPowerUsage     = this->realPowerUsageOrig / 1000.0;
            }
        } else if (element.key() == "refTriggerStamp") {
            if (element.value() == 0) {
                return;
            }
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = refTrigger_ns / std::pow(10, 9);
        }
    }

    lastTimeStamp        = lastRefTrigger;
    this->lastRefTrigger = this->refTrigger_ns;

    this->init           = true;
    this->success        = true;
}

void RealPowerUsage::fail() {
    this->success = false;
}

template class IAcquisition<Buffer>;
template class IAcquisition<ScrollingBuffer>;
