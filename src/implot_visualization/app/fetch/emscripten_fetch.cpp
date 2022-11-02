#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten_fetch.h>
#include <format>
#include <iostream>
#include <string.h>

void Subscription::downloadSucceeded(emscripten_fetch_t *fetch) {
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    deserializer.jsonString.assign(fetch->data, fetch->numBytes);
    fetchSuccessful = true;

    emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void Subscription::downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
    fetchFinished = true;
}

void onDownloadSucceeded(emscripten_fetch_t *fetch) {
    Subscription *fetchUtils = static_cast<Subscription *>(fetch->userData);
    fetchUtils->downloadSucceeded(fetch);
}

void onDownloadFailed(emscripten_fetch_t *fetch) {
    Subscription *fetchUtils = static_cast<Subscription *>(fetch->userData);
    fetchUtils->downloadFailed(fetch);
}

Subscription::Subscription(const std::string _url, const std::vector<std::string> &_requestedSignals) {
    url              = _url;
    requestedSignals = _requestedSignals;
    for (std::string str : _requestedSignals) {
        url = url + str + ",";
    }
    if (!url.empty()) {
        url.pop_back();
    }

    int                       numSignals = _requestedSignals.size();
    std::vector<SignalBuffer> _signals(numSignals);
    signals     = _signals;

    extendedUrl = url + "&lastRefTrigger=0";
}

void Subscription::fetch() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    // static const char *custom_headers[3] = { "X-OPENCMW-METHOD", "POLL", nullptr };
    // attr.requestHeaders = custom_headers;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess  = onDownloadSucceeded;
    attr.onerror    = onDownloadFailed;
    attr.userData   = this;

    if (fetchFinished) {
        emscripten_fetch(&attr, extendedUrl.c_str());
        fetchFinished = false;
    }
    if (fetchSuccessful) {
        deserializer.deserializeJson(signals);
        updateUrl();
        fetchSuccessful = false;
        fetchFinished   = true;
    }
}

void Subscription::updateUrl() {
    extendedUrl = url + "&lastRefTrigger=" + std::to_string(deserializer.lastRefTrigger);
}