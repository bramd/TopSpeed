/**
* Common library - Emscripten/WASM stubs
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#ifdef __EMSCRIPTEN__

#include <Common/If/Common.h>
#include <cstdlib>
#include <cstring>
#include <cctype>

// =============================================================================
// Utility functions from Common.cpp
// =============================================================================

char* strLower(char* s)
{
    for (char* p = s; *p; ++p)
        *p = tolower(*p);
    return s;
}

char* strUpper(char* s)
{
    for (char* p = s; *p; ++p)
        *p = toupper(*p);
    return s;
}

int intFromString(const char* s)
{
    return atoi(s);
}

char* stringFromInt(int i)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "%d", i);
    return buf;
}

// =============================================================================
// Mutex stubs (single-threaded in WASM)
// =============================================================================

Mutex::Mutex()
{
    // No-op in single-threaded WASM
}

Mutex::~Mutex()
{
    // No-op
}

void Mutex::enter()
{
    // No-op in single-threaded WASM
}

void Mutex::leave()
{
    // No-op in single-threaded WASM
}

// =============================================================================
// Network stubs (not supported in WASM)
// =============================================================================

char* getLocalIpAddress()
{
    static char ip[] = "127.0.0.1";
    return ip;
}

// =============================================================================
// Global tracer instance (from Common.cpp)
// =============================================================================

Tracer commonTracer("COMMON");

#endif // __EMSCRIPTEN__
