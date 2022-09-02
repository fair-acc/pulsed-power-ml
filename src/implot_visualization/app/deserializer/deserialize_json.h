#pragma once

#include <imgui.h>
#include <iostream>
#include <shared_mutex>
#include <vector>

struct DataPoint {
    double x;
    double y;

    constexpr DataPoint();
    constexpr DataPoint(double _x, double _y);
};

struct ScrollingBuffer {
    int                 MaxSize;
    int                 Offset;
    ImVector<DataPoint> Data;
    std::string         Name;

    ScrollingBuffer(int max_size = 200'000);

    void AddPoint(double x, double y);
    /**
     * @brief Adds a new vector to the internal buffer.
     * @remarks This method is not thread-safe.
     *
     * @param x The x locations of the point.
     * @param y The y locations of the point.
     */
    void AddVector(const std::vector<double> &x, const std::vector<double> &y);
    void Erase();
};

/**
 * @brief Deserializes json string
 *
 * @param jsonString string of type
 * "Acquisition": {"value": key, "timestamp": key}
 */
void deserializeJson(const std::string &jsonString, ScrollingBuffer &buffer);