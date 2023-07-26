#pragma once

#include <stdint.h>

namespace Platform
{
    bool FileExists(const char* path);
    int64_t LastModified(const char* path);
    void* DllOpen(const char* path);
    int DllClose(void* handle);
    void* GetSymbol(void* handle, const char* name);
}
