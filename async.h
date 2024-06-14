#pragma once

#include <cinttypes>
void* connect(std::size_t N);
void receive(const char * data, std::size_t dataSize, void* context);
void disconnect(void* context);
