#pragma once

#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>

void downloadSucceeded(emscripten_fetch_t *fetch, ScrollingBuffer &buffer);
void downloadFailed(emscripten_fetch_t *fetch);
void fetch(const char *url, ScrollingBuffer &buffer);