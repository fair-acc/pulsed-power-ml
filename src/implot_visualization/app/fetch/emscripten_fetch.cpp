#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten_fetch.h>
#include <format>
#include <iostream>
#include <string.h>

std::string jsonString;

void        FetchUtils::downloadSucceeded(emscripten_fetch_t *fetch) {
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    jsonString.assign(fetch->data, fetch->numBytes);
    fetchSuccessful = true;

    emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void FetchUtils::downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
    fetchFinished = true;
}

void onDownloadSucceeded(emscripten_fetch_t *fetch) {
    FetchUtils *fetchUtils = static_cast<FetchUtils *>(fetch->userData);
    fetchUtils->downloadSucceeded(fetch);
}

void onDownloadFailed(emscripten_fetch_t *fetch) {
    FetchUtils *fetchUtils = static_cast<FetchUtils *>(fetch->userData);
    fetchUtils->downloadFailed(fetch);
}

FetchUtils::FetchUtils(const char *_url, const int numSignals) {
    url = _url;
    std::vector<SignalBuffer> _signals(numSignals);
    signals     = _signals;
    extendedUrl = std::format("{}&lastRefTrigger=0", url);
}

void FetchUtils::fetch() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    static const char *custom_headers[3] = { "X-OPENCMW-METHOD", "POLL", nullptr };
    attr.requestHeaders                  = custom_headers;
    attr.attributes                      = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess                       = onDownloadSucceeded;
    attr.onerror                         = onDownloadFailed;
    attr.userData                        = this;

    if (fetchFinished) {
        emscripten_fetch(&attr, extendedUrl.c_str());
        fetchFinished = false;
    }
    if (fetchSuccessful) {
        deserializer.deserializeJson(jsonString, signals);
        updateUrl();
        fetchSuccessful = false;
        fetchFinished   = true;
    }
}

void FetchUtils::updateUrl() {
    extendedUrl = std::format("{}&lastRefTrigger={}", url, std::to_string(deserializer.lastRefTrigger));
}