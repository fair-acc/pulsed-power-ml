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

ScrollingBuffer::ScrollingBuffer(int max_size) {
    MaxSize = max_size;
    Offset  = 0;
    Data.reserve(MaxSize);
}

void ScrollingBuffer::AddPoint(double x, double y) {
    if (Data.size() < MaxSize)
        Data.push_back(DataPoint(x, y));
    else {
        Data[Offset] = DataPoint(x, y);
        Offset       = (Offset + 1) % MaxSize;
    }
}

void ScrollingBuffer::AddVector(const std::vector<double> &x, const std::vector<double> &y) {
    for (int i = 0; i < x.size(); i++) {
        AddPoint(x[i], y[i]);
    }
}

void ScrollingBuffer::Erase() {
    if (Data.size() > 0) {
        Data.shrink(0);
        Offset = 0;
    }
}

void deserializeAcquisition(const std::string &jsonString, ScrollingBuffer &buffer) {
    uint64_t                 refTrigger_ns = 0;
    double                   refTrigger_s  = 0.0;
    std::vector<std::string> signalNames;
    std::vector<double>      values;
    std::vector<double>      relativeTimestamps;

    // TODO: dataAsJson is given in format R"("Acquisition": {"value": 27 })" which
    // cannot be read by json::parse(). Maybe opencmw::serealiserJson is the better
    // deserializer.
    // For now, remove ""Acquisition": " from dataAsJson to make it usable with
    // json::parse
    std::string modifiedJsonString = jsonString;
    modifiedJsonString.erase(0, 14);

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            refTrigger_ns = element.value();
            refTrigger_s  = refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            relativeTimestamps.insert(relativeTimestamps.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelNames") {
            signalNames.insert(signalNames.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelValues") {
            values.insert(values.begin(), element.value().begin(), element.value().end());
        }
    }

    std::vector<double> timestamps;
    for (auto dt : relativeTimestamps) {
        timestamps.push_back(refTrigger_s + dt);
    }

    buffer.AddVector(timestamps, values);
}

void deserializeCounter(const std::string &jsonString, ScrollingBuffer &buffer) {
    double timestamp = 0.0;
    double value     = 0.0;

    // TODO: dataAsJson is given in format R"("CounterData": {"value": 27 })" which
    // cannot be read by json::parse(). Maybe opencmw::serealiserJson is the better
    // deserializer.
    // For now, remove ""CounterData": " from dataAsJson to make it usable with
    // json::parse
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
        buffer.AddPoint(timestamp, value);
    }

    tmp_t_prev = timestamp;
}

void deserializeJson(const std::string &jsonString, ScrollingBuffer &buffer) {
    if (jsonString.substr(1, 11) == "Acquisition") {
        deserializeAcquisition(jsonString, buffer);
    } else if (jsonString.substr(1, 11) == "CounterData") {
        deserializeCounter(jsonString, buffer);
    }
}