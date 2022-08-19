#pragma once

#include <emscripten.h>
#include <emscripten/fetch.h>

void downloadSucceeded(emscripten_fetch_t *fetch);
void downloadFailed(emscripten_fetch_t *fetch);
void fetch(const char *url);