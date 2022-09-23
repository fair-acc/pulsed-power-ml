#pragma once

#include <deserialize_json.h>
#include <emscripten.h>
#include <emscripten/fetch.h>

void fetch(const char *url, std::vector<SignalBuffer> &signals, Deserializer *deserializer);
void downloadSucceeded(emscripten_fetch_t *fetch);
void downloadFailed(emscripten_fetch_t *fetch);
