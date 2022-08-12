#pragma once

#include <imgui.h>
#include <iostream>

struct DataVector {
    int64_t x;
    int64_t y;

    constexpr DataVector();
    constexpr DataVector(int64_t _x, int64_t _y);

    int64_t  operator[](size_t idx) const;
    int64_t &operator[](size_t idx);
};

// Utility structure for realtime plot
struct ScrollingBuffer {
    int                  MaxSize;
    int                  Offset;
    ImVector<DataVector> Data;

    ScrollingBuffer(int max_size = 2000);

    void AddPoint(int64_t x, int64_t y);
    void Erase();
};

// Deserialise
// dataAsJson:  String of format {"key1": value1, "key2": value2, ...}
void deserialiseJson(std::string jsonString);