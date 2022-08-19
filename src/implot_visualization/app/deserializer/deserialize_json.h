#pragma once

#include <imgui.h>
#include <iostream>
#include <shared_mutex>

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
    std::mutex           WriteLock;
    ImVector<DataVector> Data;

    ScrollingBuffer(int max_size = 2000);

    /**
     * @brief Adds a new point to the internal buffer.
     * @remarks This method is not thread-safe.
     *
     * @param x The x location of the point.
     * @param y The y location of the point.
     */
    void AddPoint(int64_t x, int64_t y);
    void Erase();
};

// Deserialise
// dataAsJson:  String of format {"key1": value1, "key2": value2, ...}
void deserialiseJson(const std::string &jsonString);