#include <cmath>
#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern ScrollingBuffer buffer;
uint64_t               prev_refTrigger = 0;

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
    if (x.size() != y.size()) {
        std::cout << "The number of timestamps does not match the number of signal values." << std::endl;
        return;
    }
    for (int i = 0; i < x.size(); i++) {
        buffer.AddPoint(x[i], y[i]);
        // std::cout << "[x: " << x[i] << ", y: " << y[i] << "],";
    }
    // std::cout << std::endl;
}

void ScrollingBuffer::Erase() {
    if (Data.size() > 0) {
        Data.shrink(0);
        Offset = 0;
    }
}

void deserialiseJson(const std::string &jsonString) {
    uint64_t                 refTrigger_ns;
    double                   refTrigger_s;
    std::vector<std::string> signalNames;
    std::vector<double>      values;
    std::vector<double>      relativeTimestamps;
    std::string              name = "Sinus";

    std::cout.precision(19);

    // TODO: dataAsJson is given in format R"("Acquisition": {"value": 27 })" which
    // cannot be read by json::parse(). Maybe opencmw::serealiserJson is the better
    // deserialiser.
    // For now, remove ""Acquisition": " from dataAsJson to make it usable with
    // json::parse
    std::string modifiedJsonString = jsonString;
    modifiedJsonString.erase(0, 14);

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        // if (element.key() == "refTriggerName") {
        //     name = element.value();
        // }
        if (element.key() == "refTriggerStamp") {
            refTrigger_ns = element.value();
            refTrigger_s  = refTrigger_ns / std::pow(10, 9);
            if (refTrigger_ns == prev_refTrigger) {
                // std::cout << "refTrigger == previous" << std::endl;
                // std::cout << refTrigger_ns << std::endl;
                return;
            }
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            relativeTimestamps.insert(relativeTimestamps.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelNames") {
            signalNames.insert(signalNames.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelValues") {
            values.insert(values.begin(), element.value().begin(), element.value().end());
        }
        // std::cout << "Continue extraction" << std::endl;
    }

    if (refTrigger_ns != prev_refTrigger) {
        std::vector<double> timestamps;
        for (auto dt : relativeTimestamps) {
            timestamps.push_back(refTrigger_s + dt);
        }

        std::cout << "Absolute timestamps: " << timestamps[0] << std::endl;

        buffer.AddVector(timestamps, values);
    }

    prev_refTrigger = refTrigger_ns;
}