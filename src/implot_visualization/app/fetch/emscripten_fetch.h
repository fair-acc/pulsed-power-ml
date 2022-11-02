#pragma once

#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>

class Subscription {
public:
    std::string               url;
    std::vector<SignalBuffer> signals;
    bool                      fetchFinished   = true;
    bool                      fetchSuccessful = false;

    Subscription(const std::string _url, const std::vector<std::string> &_requestedSignals);

    void fetch();
    void downloadSucceeded(emscripten_fetch_t *fetch);
    void downloadFailed(emscripten_fetch_t *fetch);

private:
    std::vector<std::string> requestedSignals;
    Deserializer             deserializer;
    std::string              extendedUrl;

    void                     updateUrl();
};
