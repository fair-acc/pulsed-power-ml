#include <cmath>
#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern ScrollingBuffer buffer;
uint64_t               prev_refTrigger = 0;
float                  counter         = 0.0f;

constexpr DataVector::DataVector()
    : x(0.0f), y(0.0f) {}
constexpr DataVector::DataVector(float _x, float _y)
    : x(_x), y(_y) {}

ScrollingBuffer::ScrollingBuffer(int max_size) {
    MaxSize = max_size;
    Offset  = 0;
    Data.reserve(MaxSize);
}

void ScrollingBuffer::AddPoint(float x, float y) {
    if (Data.size() < MaxSize)
        Data.push_back(DataVector(x, y));
    else {
        Data[Offset] = DataVector(x, y);
        Offset       = (Offset + 1) % MaxSize;
    }
}

void ScrollingBuffer::AddVector(std::vector<float> x, std::vector<float> y) {
    if (x.size() != y.size()) {
        std::cout << "The number of timestamps does not match the number of signal values." << std::endl;
        return;
    }
    // for debug purposes
    for (int i = 0; i < x.size(); i++) {
        buffer.AddPoint(x[i], y[i]);
        std::cout << "[x: " << x[i] << ", y: " << y[i] << "],";
    }
    std::cout << std::endl;
}
void ScrollingBuffer::Erase() {
    if (Data.size() > 0) {
        Data.shrink(0);
        Offset = 0;
    }
}

void deserialiseJson(const std::string &jsonString) {
    uint64_t                 timestamp;
    std::vector<std::string> signalNames;
    std::vector<float>       values;
    std::vector<float>       relativeTimestamps;
    std::string              name = "Sinus";

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
            timestamp = element.value();
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            relativeTimestamps.insert(relativeTimestamps.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelNames") {
            signalNames.insert(signalNames.begin(), element.value().begin(), element.value().end());
        } else if (element.key() == "channelValues") {
            values.insert(values.begin(), element.value().begin(), element.value().end());
        }
    }
    std::cout << "Json deserialization finished." << std::endl;

    if (timestamp != prev_refTrigger) {
        std::vector<float> timestamps;
        for (auto dt : relativeTimestamps) {
            // timestamps.push_back(timestamp + dt * std::pow(10, 9));
            timestamps.push_back(counter);
            counter = counter + 1.0;
        }
        std::cout << "Absolute timestamps: " << timestamps[0] << std::endl;

        std::cout << "First element: [" << timestamps[0] << ", " << values[0] << "]" << std::endl;
        std::cout << "Last element: [" << timestamps[timestamps.size() - 1] << ", " << values[values.size() - 1] << "]" << std::endl;

        buffer.AddVector(timestamps, values);
        std::cout << "Buffer size: " << buffer.Data.size() << std::endl;
    }

    prev_refTrigger = timestamp;
}