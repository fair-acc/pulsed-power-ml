#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten_fetch.h>
#include <iostream>
#include <string.h>

volatile bool fetch_finished   = true;
bool          fetch_successful = false;
std::string   jsonString;

void          downloadSucceeded(emscripten_fetch_t *fetch) {
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    fetch_successful = true;
    std::string jsonData(fetch->data, fetch->data + fetch->numBytes);
    jsonString = jsonData;

    emscripten_fetch_close(fetch); // Free data associated with the fetch.
    fetch_finished = true;
}

void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
    fetch_finished = true;
}

bool fetch(const char *url, std::vector<SignalBuffer> &signals, Deserializer *deserializer) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    static const char *custom_headers[3] = { "X-OPENCMW-METHOD", "POLL", nullptr };
    attr.requestHeaders                  = custom_headers;
    attr.attributes                      = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess                       = downloadSucceeded;
    attr.onerror                         = downloadFailed;

    // // synchronous fetch emscripten
    // attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
    // std::cout << url << std::endl;
    // emscripten_fetch_t *fetch = emscripten_fetch(&attr, url);
    // if (fetch->status == 200) {
    //     printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
    // } else {
    //     printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    // }

    // own synchronous fetch
    if (fetch_finished) {
        emscripten_fetch(&attr, url);
        fetch_finished = false;
    }
    if (fetch_successful) {
        deserializer->deserializeJson(jsonString, signals);
        fetch_successful = false;
    }
    return fetch_finished;
}