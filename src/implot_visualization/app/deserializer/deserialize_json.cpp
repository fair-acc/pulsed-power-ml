#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern ScrollingBuffer buffer;
long                   tmp_x_prev;
long                   tmp_x_first;

constexpr DataVector::DataVector()
    : x(0.0f), y(0.0f) {}
constexpr DataVector::DataVector(long _x, long _y)
    : x(_x), y(_y) {}
long DataVector::operator[](size_t idx) const {
    IM_ASSERT(idx <= 1);
    return (&x)[idx];
}
long &DataVector::operator[](size_t idx) {
    IM_ASSERT(idx <= 1);
    return (&x)[idx];
}

ScrollingBuffer::ScrollingBuffer(int max_size) {
    MaxSize = max_size;
    Offset  = 0;
    Data.reserve(MaxSize);
}
void ScrollingBuffer::AddPoint(long x, long y) {
    if (Data.size() < MaxSize)
        Data.push_back(DataVector(x, y));
    else {
        Data[Offset] = DataVector(x, y);
        Offset       = (Offset + 1) % MaxSize;
    }
}
void ScrollingBuffer::Erase() {
    if (Data.size() > 0) {
        Data.shrink(0);
        Offset = 0;
    }
}

// Deserialise
// dataAsJson:  String of format {"key1": value1, "key2": value2, ...}
void deserialiseJson(std::string jsonString) {
    long tmp_x;
    int  tmp_y;

    // TODO: dataAsJson is given in format R"("CounterData": {"value": 27 })" which
    // cannot be read by json::parse(). Maybe opencmw::serealiserJson is the better
    // deserialiser.
    // For now, remove ""CounterData": " from dataAsJson to make it usable with
    // json::parse
    std::string modifiedJsonString = jsonString.erase(0, 14);
    // std::cout << "Modified Json string: " << modifiedJsonString << "\n";

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        // std::cout << "Json elements: key: " << element.key() << ", value: " << element.value() << "\n";
        if (element.key() == "timestamp") {
            tmp_x = element.value();
        } else {
            tmp_y = element.value();
        }
    }
    std::cout << "Long: " << std::setprecision(std::numeric_limits<float>::digits) << tmp_x << ", y: " << tmp_y << "\n";
    std::cout << "Float: " << std::setprecision(std::numeric_limits<float>::digits) << static_cast<float>(tmp_x) << ", y: " << tmp_y << "\n";

    if (tmp_x != tmp_x_prev) {
        buffer.AddPoint(tmp_x, tmp_y);
        // For debug purpose
        for (auto element : buffer.Data) {
            std::cout << "[" << element.x << ", " << element.y << "] , ";
        }
        std::cout << "\n";
    }

    // printf("Deserialisation finished.\n");
    tmp_x_prev = tmp_x;
}