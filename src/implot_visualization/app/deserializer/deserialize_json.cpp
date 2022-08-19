#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>

using json = nlohmann::json;

extern ScrollingBuffer buffer;
int64_t                tmp_x_prev;
int64_t                tmp_x_first;

constexpr DataVector::DataVector()
    : x(0.0f), y(0.0f) {}
constexpr DataVector::DataVector(int64_t _x, int64_t _y)
    : x(_x), y(_y) {}
int64_t DataVector::operator[](size_t idx) const {
    IM_ASSERT(idx <= 1);
    return idx == 0 ? x : y;
}
int64_t &DataVector::operator[](size_t idx) {
    IM_ASSERT(idx <= 1);
    return idx == 0 ? x : y;
}

ScrollingBuffer::ScrollingBuffer(int max_size) {
    MaxSize = max_size;
    Offset  = 0;
    Data.reserve(MaxSize);
}
void ScrollingBuffer::AddPoint(int64_t x, int64_t y) {
    // std::lock_guard<std::mutex> guard(WriteLock);
    std::cout << "Starting AddPoint " << std::this_thread::get_id() << std::endl;
    if (Data.size() < MaxSize)
        Data.push_back(DataVector(x, y));
    else {
        Data[Offset] = DataVector(x, y);
        Offset       = (Offset + 1) % MaxSize;
    }
    std::cout << "Ending AddPoint " << std::this_thread::get_id() << std::endl;
}
void ScrollingBuffer::Erase() {
    if (Data.size() > 0) {
        Data.shrink(0);
        Offset = 0;
    }
}

// Deserialise
// dataAsJson:  String of format {"key1": value1, "key2": value2, ...}
void deserialiseJson(const std::string &jsonString) {
    long tmp_x;
    int  tmp_y;

    // TODO: dataAsJson is given in format R"("CounterData": {"value": 27 })" which
    // cannot be read by json::parse(). Maybe opencmw::serealiserJson is the better
    // deserialiser.
    // For now, remove ""CounterData": " from dataAsJson to make it usable with
    // json::parse
    std::string modifiedJsonString = jsonString;
    modifiedJsonString.erase(0, 14);
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

    if (tmp_x != tmp_x_prev) {
        buffer.AddPoint(tmp_x, tmp_y);
        // For debug purposes
        // for (auto element : buffer.Data) {
        //     std::cout << "[" << element.x << ", " << element.y << "] , ";
        // }
        // std::cout << "\n";
    }

    // printf("Deserialisation finished.\n");
    tmp_x_prev = tmp_x;
}