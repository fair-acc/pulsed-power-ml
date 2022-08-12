#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten_fetch.h>
#include <iostream>
#include <string.h>

extern bool fetch_finished;

void        downloadSucceeded(emscripten_fetch_t *fetch) {
    // printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    std::string jsonData;
    for (int i = 0; i < fetch->numBytes; i++) {
        jsonData += fetch->data[i];
    }

    // printf("Json string:\n%s\n", jsonData.c_str());
    deserialiseJson(jsonData.c_str());
    emscripten_fetch_close(fetch); // Free data associated with the fetch.
    fetch_finished = true;
}

void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
    fetch_finished = true;
}