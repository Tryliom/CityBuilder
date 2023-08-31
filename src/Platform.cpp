#include "Platform.h"

#include <assert.h>
#include <stdio.h>

#if defined(_WIN32)
    #include <malloc.h> // NOTE: on unix there is no malloc.h, it's in stdlib or something
    #include <windows.h>
    #include <shlwapi.h> // Include this header for PathCombine function
    #pragma comment(lib, "Shlwapi.lib") // Link to the Shlwapi library
#elif defined(__APPLE__)
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dlfcn.h>
#endif

namespace Platform
{
    #ifdef _WIN32

    bool FileExists(const char* path) 
    {
        return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
    }

    int64_t LastModified(const char* path) 
    {
        if (!FileExists(path)) 
        {
            return -1;
        }

        FILETIME ftLastWriteTime;

        // Open the file, specifying the desired access rights, sharing mode, and other parameters.
        HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE && GetFileTime(hFile, NULL, NULL, &ftLastWriteTime)) 
        {
            ULARGE_INTEGER uli;
            uli.LowPart  = ftLastWriteTime.dwLowDateTime;
            uli.HighPart = ftLastWriteTime.dwHighDateTime;
            CloseHandle(hFile);

            return static_cast<int64_t>(uli.QuadPart / 10000000ULL - 11644473600ULL);
        }

        return -1;
    }

    #else



    int64_t LastModified(const char* path) 
    {
        struct stat info;
        int err = stat(path, &info);
        assert(err == 0);

        //struct timespec time = info.st_mtimespec;
        time_t time = info.st_mtime; // From some documentation: "For historical reasons, it is generally implemented as an integral value representing the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC(i.e., a unix timestamp). Although libraries may implement this type using alternative time representations.

        return (uint64_t) time;
    }

    #endif

    void* DllOpen(const char* path)
    {
        #if defined(_WIN32)
            HINSTANCE hInst;
            hInst = LoadLibraryA(path);
            if (hInst==NULL) {
                printf("DllOpen error: %s\n", GetLastError());
                exit(-1);
            }
            return hInst;
        #elif defined(__APPLE__) || defined(LINUX)
            void* lib = dlopen(path, RTLD_LOCAL | RTLD_NOW);
            if (lib == NULL) {
                printf("Couldn't load dynamic lib '%s': %s\n", path, dlerror());
                abort();
            }
            return lib;
        #endif
    }

    int DllClose(void* handle)
    {
        #ifdef _WIN32
        int rc = 0;
        BOOL ok;
        ok = FreeLibrary((HINSTANCE)handle);
        if (!ok) {
            printf("PlatformDllClose error: %s\n", GetLastError());
            //var.lasterror = GetLastError();
            //var.err_rutin = "dlclose";
            rc = -1;
        }
        return rc;
        #elif defined(__APPLE__) || defined(LINUX)
        return dlclose(handle);
        #endif
    }

    void* GetSymbol(void* handle, const char* name)
    {
        #ifdef _WIN32
        FARPROC fp;

        fp = GetProcAddress((HINSTANCE)handle, name);
        if (!fp) {
            printf("PlatformGetSymbol error: %s\n", GetLastError());
        }
        return (void *)(intptr_t)fp;
        #elif defined(__APPLE__) || defined(LINUX)
        return dlsym(handle, name);
        #endif
    }
}
