#pragma once

#include <imgui.h>
#include <iostream>
#include <shared_mutex>
#include <vector>

struct DataVector {
    float x;
    float y;

    constexpr DataVector();
    constexpr DataVector(float _x, float _y);
};

struct ScrollingBuffer {
    int                  MaxSize;
    int                  Offset;
    ImVector<DataVector> Data;
    std::string          Name;

    ScrollingBuffer(int max_size = 10000);

    void AddPoint(float x, float y);
    /**
     * @brief Adds a new vector to the internal buffer.
     * @remarks This method is not thread-safe.
     *
     * @param x The x locations of the point.
     * @param y The y locations of the point.
     */
    void AddVector(std::vector<float> x, std::vector<float> y);
    void Erase();
};

/**
 * @brief Deserializes json string
 *
 * @param jsonString string of type
 * "Acquisition": {"value": key, "timestamp": key}
 */
void deserialiseJson(const std::string &jsonString);