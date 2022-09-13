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

Signal::Signal(int max_size) {
    MaxSize = max_size;
    Offset  = 0;
    Data.reserve(MaxSize);
}

void Signal::AddPoint(double x, double y) {
    if (Data.size() < MaxSize)
        Data.push_back(DataPoint(x, y));
    else {
        Data[Offset] = DataPoint(x, y);
        Offset       = (Offset + 1) % MaxSize;
    }
}

void Signal::Erase() {
    if (Data.size() > 0) {
        Data.shrink(0);
        Offset = 0;
    }
}

void addToSignalBuffers(std::vector<Signal> &signals,
        std::vector<std::string>             signalNames,
        double                               refTrigger,
        std::vector<double>                 &relativeTimestamps,
        std::vector<int>                     dims,
        std::vector<double>                 &strideArray) {
    double absoluteTimestamp = 0.0;
    double value             = 0.0;

    for (int i = 0; i < signalNames.size(); i++) {
        signals[i].Name = signalNames[i];
        int stride      = dims[1];
        int offset      = i * stride;

        for (int j = 0; j < relativeTimestamps.size(); j++) {
            absoluteTimestamp = refTrigger + relativeTimestamps[j];
            value             = strideArray[offset + j];
            signals[i].AddPoint(absoluteTimestamp, value);
        }
    }
}

void deserializeAcquisition(const std::string &jsonString, std::vector<Signal> &signals) {
    uint64_t                 refTrigger_ns = 0;
    double                   refTrigger_s  = 0.0;
    std::vector<std::string> signalNames;
    std::vector<double>      relativeTimestamps;
    std::vector<int>         dims;
    std::vector<double>      strideArray;

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
            dims        = std::vector<int>(element.value()["dims"]);
            strideArray = std::vector<double>(element.value()["values"]);
            std::cout << "StrideArray: " << strideArray.size() << std::endl;
        }
    }

    addToSignalBuffers(signals, signalNames, refTrigger_s, relativeTimestamps, dims, strideArray);
}

void deserializeCounter(const std::string &jsonString, std::vector<Signal> &signals) {
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
        signals[0].AddPoint(timestamp, value);
    }

    tmp_t_prev = timestamp;
}

void deserializeJson(const std::string &jsonString, std::vector<Signal> &signals) {
    if (jsonString.substr(1, 11) == "Acquisition") {
        deserializeAcquisition(jsonString, signals);
    } else if (jsonString.substr(1, 11) == "CounterData") {
        deserializeCounter(jsonString, signals);
    }
}