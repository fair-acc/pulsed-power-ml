#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten_fetch.h>
#include <format>
#include <iostream>
#include <string.h>

template<typename T>
void Subscription<T>::downloadSucceeded(emscripten_fetch_t *fetch) {
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    this->acquisition.jsonString.assign(fetch->data, fetch->numBytes);

    emscripten_fetch_close(fetch); // Free data associated with the fetch.
    this->fetchSuccessful     = true;
    this->acquisition.success = true;
}

template<typename T>
void Subscription<T>::downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
    this->fetchFinished       = true;
    this->acquisition.success = false;
}

template<typename T>
void onDownloadSucceeded(emscripten_fetch_t *fetch) {
    Subscription<T> *sub = static_cast<Subscription<T> *>(fetch->userData);
    sub->downloadSucceeded(fetch);
}

template<typename T>
void onDownloadFailed(emscripten_fetch_t *fetch) {
    Subscription<T> *sub = static_cast<Subscription<T> *>(fetch->userData);
    sub->downloadFailed(fetch);
}

template<typename T>
std::string Subscription<T>::buildSignalString(const std::string name, const double sampRate) {
    std::string sampRateStr = std::to_string(sampRate);
    sampRateStr.erase(sampRateStr.find_last_not_of('0') + 1, std::string::npos);
    sampRateStr.erase(sampRateStr.find_last_not_of('.') + 1, std::string::npos);
    return name + "@" + sampRateStr + "Hz";
}

template<typename T>
Subscription<T>::Subscription(const std::string &_url, const std::vector<std::string> &_requestedSignals, const double _sampRate, const int _bufferSize, const float _updateFrequency)
    : url(_url), updateFrequency(_updateFrequency) {
    for (std::string signalName : _requestedSignals) {
        if (_sampRate == 0) {
            requestedSignals.push_back(signalName);
            this->url = this->url + signalName + ",";
        } else {
            std::string fullSignalName = buildSignalString(signalName, _sampRate);
            requestedSignals.push_back(fullSignalName);
            this->url = this->url + fullSignalName + ",";
        }
    }
    if (!this->url.empty()) {
        this->url.pop_back();
    }

    T _acquisition(requestedSignals, _bufferSize);
    this->acquisition = _acquisition;

    if (url.find("channelNameFilter") != std::string::npos) {
        this->extendedUrl = this->url + "&lastRefTrigger=0";
    } else {
        this->extendedUrl = this->url;
    }

    auto   clock        = std::chrono::system_clock::now();
    double currentTime  = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    this->lastFetchTime = currentTime;
}

template<typename T>
void Subscription<T>::fetch() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    // static const char *custom_headers[3] = { "X-OPENCMW-METHOD", "POLL", nullptr };
    // attr.requestHeaders = custom_headers;
    attr.attributes    = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess     = onDownloadSucceeded<T>;
    attr.onerror       = onDownloadFailed<T>;
    attr.userData      = this;

    auto   clock       = std::chrono::system_clock::now();
    double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    if (currentTime - this->lastFetchTime >= 1.0f / this->updateFrequency) {
        if (this->fetchFinished) {
            this->fetchFinished = false;
            emscripten_fetch(&attr, this->extendedUrl.c_str());
        }
        if (fetchSuccessful) {
            this->acquisition.deserialize();
            if (url.find("channelNameFilter") != std::string::npos) {
                updateUrl();
            }
            this->fetchSuccessful = false;
            this->fetchFinished   = true;
            this->lastFetchTime   = currentTime;
        }
    }
}

template<typename T>
void Subscription<T>::updateUrl() {
    this->extendedUrl = this->url + "&lastRefTrigger=" + std::to_string(acquisition.lastTimeStamp);
}

template class Subscription<Acquisition>;
template class Subscription<AcquisitionSpectra>;
template class Subscription<PowerUsage>;
template class Subscription<RealPowerUsage>;
template class Subscription<PowerUsageWeek>;
