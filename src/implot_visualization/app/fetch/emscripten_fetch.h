#pragma once

#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>

class FetchUtils {
public:
    const char               *url;
    std::vector<SignalBuffer> signals;
    volatile bool             fetchFinished   = true;
    volatile bool             fetchSuccessful = false;

    FetchUtils(const char *_url, int numSignals);

    void fetch();
    void downloadSucceeded(emscripten_fetch_t *fetch);
    void downloadFailed(emscripten_fetch_t *fetch);

private:
    Deserializer deserializer;
    std::string  extendedUrl;

    void         updateUrl();
};
