#pragma once 

#include "iostream"

// Object that contain a memory block.
struct Span
{
    uint8_t* ptr;
    size_t size;
};

namespace FileSystem
{
    inline Span loadEntireFile(const char* filePath)
    {
        Span span{};

        FILE* file = fopen(filePath, "rb");
        fseek(file, 0, SEEK_END); // Put the reader cursor at the end of the file. 
        span.size = ftell(file);  // Return the cursor emplacement (here it is at the end of the file because we want to know the size of the file).
        fseek(file, 0, SEEK_SET); // Set the cursor where we want (here 0 so the begining of the file).

        span.ptr = (uint8_t*)malloc(span.size);
        fread(span.ptr, span.size, 1, file);
        fclose(file);

        return span;
    }
};

