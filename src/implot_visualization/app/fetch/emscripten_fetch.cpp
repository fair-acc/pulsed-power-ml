#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten_fetch.h>
#include <iostream>
#include <string.h>

bool fetch_finished = true;

void downloadSucceeded(emscripten_fetch_t *fetch) {
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    std::string jsonData(fetch->data, fetch->data + fetch->numBytes);
    deserialiseJson(jsonData.c_str());

    emscripten_fetch_close(fetch); // Free data associated with the fetch.
    fetch_finished = true;
}

void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
    fetch_finished = true;
}

void fetch(const char *url) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    static const char *custom_headers[3] = { "X-OPENCMW-METHOD", "POLL", nullptr };
    attr.requestHeaders                  = custom_headers;
    attr.attributes                      = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess                       = downloadSucceeded;
    attr.onerror                         = downloadFailed;
    if (fetch_finished) {
        emscripten_fetch(&attr, url);
        fetch_finished = false;
    }
}