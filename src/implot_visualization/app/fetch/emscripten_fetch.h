#pragma once

#include <chrono>
#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>

template<typename T>
class Subscription {
public:
    std::string    url;
    T              acquisition;
    std::vector<T> signals;
    bool           fetchFinished   = true;
    bool           fetchSuccessful = false;

    Subscription(const std::string &_url, const std::vector<std::string> &_requestedSignals, const double _sampRate, const int _bufferSize, const float _updateFrequency);

    void fetch();
    void downloadSucceeded(emscripten_fetch_t *fetch);
    void downloadFailed(emscripten_fetch_t *fetch);

private:
    std::vector<std::string> requestedSignals;
    std::string              extendedUrl;
    float                    updateFrequency;
    double                   lastFetchTime;

    void                     updateUrl();
    std::string              buildSignalString(const std::string name, const double sampRate);
};
