#pragma once

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
    double         lastFetchtime;


    Subscription(const std::string _url, const std::vector<std::string> &_requestedSignals);

    void fetch();
    void downloadSucceeded(emscripten_fetch_t *fetch);
    void downloadFailed(emscripten_fetch_t *fetch);

private:
    std::vector<std::string> requestedSignals;
    std::string              extendedUrl;

    void                     updateUrl();
};