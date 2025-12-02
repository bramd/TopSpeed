# CMake + SDL2 Migration Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build Top Speed 3 with VS2022/CMake using SDL2 for audio and input, with graphics stubbed and multiplayer disabled.

**Architecture:** Keep the existing library layering (Common → DxCommon → TopSpeed) but replace DirectX calls in DxCommon with SDL2 equivalents. Use preprocessor guards to conditionally compile SDL2 vs DirectX code paths, allowing incremental migration.

**Tech Stack:** CMake 3.21+, VS2022, vcpkg, SDL2, SDL2_mixer

---

## Task 1: Create Root CMake Structure

**Files:**
- Create: `CMakeLists.txt`
- Create: `CMakePresets.json`
- Create: `vcpkg.json`

**Step 1: Create vcpkg.json manifest**

```json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "topspeed",
  "version": "3.0.0",
  "dependencies": [
    "sdl2",
    {
      "name": "sdl2-mixer",
      "features": ["libvorbis"]
    }
  ]
}
```

**Step 2: Create CMakePresets.json**

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "vcpkg",
      "hidden": true,
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    },
    {
      "name": "vs2022-base",
      "hidden": true,
      "inherits": "vcpkg",
      "generator": "Visual Studio 17 2022",
      "architecture": {
        "value": "Win32",
        "strategy": "set"
      },
      "binaryDir": "${sourceDir}/build/${presetName}"
    },
    {
      "name": "vs2022-debug",
      "displayName": "VS2022 Debug",
      "inherits": "vs2022-base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "vs2022-release",
      "displayName": "VS2022 Release",
      "inherits": "vs2022-base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "vs2022-debug",
      "configurePreset": "vs2022-debug",
      "configuration": "Debug"
    },
    {
      "name": "vs2022-release",
      "configurePreset": "vs2022-release",
      "configuration": "Release"
    }
  ]
}
```

**Step 3: Create root CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.21)
project(TopSpeed VERSION 3.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 migration flags
add_compile_definitions(
    TOPSPEED_USE_SDL2=1
    TOPSPEED_DISABLE_MULTIPLAYER=1
    COMMON_STATIC
    DXCOMMON_STATIC
    _USE_VORBIS_
)

# Find dependencies
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

# Add subdirectories in dependency order
add_subdirectory(vs_projects/common)
add_subdirectory(vs_projects/dxcommon)
add_subdirectory(vs_projects/topspeed)
```

**Step 4: Verify files created**

Run: `dir CMakeLists.txt CMakePresets.json vcpkg.json`
Expected: All three files listed

**Step 5: Commit**

```bash
git add CMakeLists.txt CMakePresets.json vcpkg.json
git commit -m "Add root CMake configuration with vcpkg"
```

---

## Task 2: Create Common Library CMakeLists.txt

**Files:**
- Create: `vs_projects/common/CMakeLists.txt`

**Step 1: Create Common CMakeLists.txt**

```cmake
# Common static library
add_library(Common STATIC
    src/Algorithm.cpp
    src/Common.cpp
    src/File.cpp
    src/Mutex.cpp
    src/Network.cpp
    src/Tracer.cpp
    src/Window.cpp
)

target_include_directories(Common PUBLIC
    ${CMAKE_SOURCE_DIR}/vs_projects
)

target_compile_definitions(Common PUBLIC
    WIN32
    _LIB
    COMMON_STATIC
)

# Windows libraries needed by Common
target_link_libraries(Common PUBLIC
    Ws2_32
)
```

**Step 2: Verify file created**

Run: `type vs_projects\common\CMakeLists.txt`
Expected: CMake content displayed

**Step 3: Commit**

```bash
git add vs_projects/common/CMakeLists.txt
git commit -m "Add CMakeLists.txt for Common library"
```

---

## Task 3: Create DxCommon Library CMakeLists.txt

**Files:**
- Create: `vs_projects/dxcommon/CMakeLists.txt`

**Step 1: Create DxCommon CMakeLists.txt**

```cmake
# DxCommon static library (SDL2 backend)
add_library(DxCommon STATIC
    Src/Application.cpp
    Src/Common.cpp
    Src/D3DFont.cpp
    Src/D3DSprite.cpp
    Src/Game.cpp
    Src/Input.cpp
    Src/Light.cpp
    Src/Line.cpp
    Src/Mesh.cpp
    Src/Music.cpp
    Src/Network.cpp
    Src/Particle.cpp
    Src/Sound.cpp
    Src/Timer.cpp
    Src/Utilities.cpp
)

target_include_directories(DxCommon PUBLIC
    ${CMAKE_SOURCE_DIR}/vs_projects
)

target_compile_definitions(DxCommon PUBLIC
    WIN32
    _LIB
    DXCOMMON_STATIC
    COMMON_STATIC
    _USE_VORBIS_
    TOPSPEED_USE_SDL2
    TOPSPEED_DISABLE_MULTIPLAYER
)

target_link_libraries(DxCommon PUBLIC
    Common
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    winmm
)
```

**Step 2: Verify file created**

Run: `type vs_projects\dxcommon\CMakeLists.txt`
Expected: CMake content displayed

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/CMakeLists.txt
git commit -m "Add CMakeLists.txt for DxCommon library"
```

---

## Task 4: Create TopSpeed Executable CMakeLists.txt

**Files:**
- Create: `vs_projects/topspeed/CMakeLists.txt`

**Step 1: Create TopSpeed CMakeLists.txt**

```cmake
# TopSpeed executable
add_executable(TopSpeed WIN32
    Car.cpp
    ComputerPlayer.cpp
    Game.cpp
    Level.cpp
    LevelMultiplayer.cpp
    LevelSingleRace.cpp
    LevelTimeTrial.cpp
    Menu.cpp
    NetworkPlayer.cpp
    RaceClient.cpp
    RaceInput.cpp
    RaceServer.cpp
    RaceSettings.cpp
    StdAfx.cpp
    TopSpeed.cpp
    TopSpeedDlg.cpp
    Track.cpp
    TopSpeed.rc
)

target_include_directories(TopSpeed PRIVATE
    ${CMAKE_SOURCE_DIR}/vs_projects
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(TopSpeed PRIVATE
    WIN32
    _WINDOWS
    DXCOMMON_STATIC
    COMMON_STATIC
    _USE_VORBIS_
    TOPSPEED_USE_SDL2
    TOPSPEED_DISABLE_MULTIPLAYER
    _AFXDLL
)

# Use MFC as shared DLL
set_target_properties(TopSpeed PROPERTIES
    MFC_FLAG 2
)

target_link_libraries(TopSpeed PRIVATE
    DxCommon
    Common
)

# Precompiled header
target_precompile_headers(TopSpeed PRIVATE stdafx.h)
```

**Step 2: Verify file created**

Run: `type vs_projects\topspeed\CMakeLists.txt`
Expected: CMake content displayed

**Step 3: Commit**

```bash
git add vs_projects/topspeed/CMakeLists.txt
git commit -m "Add CMakeLists.txt for TopSpeed executable"
```

---

## Task 5: Create SDL2 Compatibility Header

**Files:**
- Create: `vs_projects/dxcommon/If/SDL2Compat.h`

**Step 1: Create SDL2 compatibility header**

```cpp
/**
 * SDL2 Compatibility Layer for DxCommon
 * Provides type definitions and macros to bridge DirectX and SDL2
 */
#ifndef __DXCOMMON_SDL2COMPAT_H__
#define __DXCOMMON_SDL2COMPAT_H__

#ifdef TOPSPEED_USE_SDL2

#include <SDL.h>
#include <SDL_mixer.h>

// Stub DirectX types when using SDL2
typedef void* LPDIRECTSOUND8;
typedef void* LPDIRECTSOUNDBUFFER;
typedef void* LPDIRECTSOUND3DBUFFER;
typedef void* LPDIRECTSOUND3DLISTENER;
typedef void* LPDIRECTINPUT8;
typedef void* LPDIRECTINPUTDEVICE8;
typedef void* LPDIRECTINPUTEFFECT;
typedef void* LPDIRECT3DDEVICE8;

// Stub DirectX structures
struct DSBUFFERDESC { int dummy; };
struct DS3DBUFFER { int dummy; };
struct DS3DLISTENER { int dummy; };
struct WAVEFORMATEX {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned long nSamplesPerSec;
    unsigned long nAvgBytesPerSec;
    unsigned short nBlockAlign;
    unsigned short wBitsPerSample;
    unsigned short cbSize;
};
typedef WAVEFORMATEX* LPWAVEFORMATEX;

struct DIDEVICEINSTANCE { int dummy; };
struct DIDEVICEOBJECTINSTANCE { int dummy; };
struct LPCDIFILEEFFECT { int dummy; };

// DirectPlay stubs (multiplayer disabled)
#ifdef TOPSPEED_DISABLE_MULTIPLAYER
typedef void* IDirectPlay8Server;
typedef void* IDirectPlay8Client;
typedef void* PDIRECTPLAY8ADDRESS;
typedef void* IDirectPlay8Address;
struct DPN_APPLICATION_DESC { int dummy; };
typedef unsigned long DPNHANDLE;
#endif

// D3D type stubs
struct D3DDISPLAYMODE { int Width; int Height; int RefreshRate; int Format; };
struct D3DPRESENT_PARAMETERS { int dummy; };
typedef int D3DFORMAT;

// D3D color macro stub
#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) ((unsigned int)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

#endif // TOPSPEED_USE_SDL2

#endif // __DXCOMMON_SDL2COMPAT_H__
```

**Step 2: Verify file created**

Run: `type vs_projects\dxcommon\If\SDL2Compat.h`
Expected: Header content displayed

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/SDL2Compat.h
git commit -m "Add SDL2 compatibility header with DirectX type stubs"
```

---

## Task 6: Modify DxCommon Internal.h for SDL2

**Files:**
- Modify: `vs_projects/dxcommon/If/Internal.h`

**Step 1: Add SDL2 conditional includes to Internal.h**

Add at the top of the file, after the `#define __DXCOMMON_INTERNAL_H__`:

```cpp
#ifdef TOPSPEED_USE_SDL2
#include <DxCommon/If/SDL2Compat.h>
#else
#include <d3d8.h>
#endif
```

Replace the existing `#include <d3d8.h>` line.

**Step 2: Verify modification**

Run: `type vs_projects\dxcommon\If\Internal.h | findstr SDL2`
Expected: Line containing SDL2Compat.h found

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/Internal.h
git commit -m "Add SDL2 conditional compilation to Internal.h"
```

---

## Task 7: Modify DxCommon Defs.h for SDL2

**Files:**
- Modify: `vs_projects/dxcommon/If/Defs.h`

**Step 1: Add SDL2 conditional to Defs.h**

Replace the `#include "d3d8types.h"` line with:

```cpp
#ifdef TOPSPEED_USE_SDL2
// D3D types are stubbed in SDL2Compat.h
#else
#include "d3d8types.h"
#endif
```

**Step 2: Verify modification**

Run: `type vs_projects\dxcommon\If\Defs.h | findstr SDL2`
Expected: SDL2 conditional found

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/Defs.h
git commit -m "Add SDL2 conditional compilation to Defs.h"
```

---

## Task 8: Modify Sound.h for SDL2

**Files:**
- Modify: `vs_projects/dxcommon/If/Sound.h`

**Step 1: Add SDL2 conditional includes to Sound.h**

Replace the DirectSound includes section (lines 10-14) with:

```cpp
#ifdef TOPSPEED_USE_SDL2
#include <DxCommon/If/SDL2Compat.h>
#include <SDL.h>
#include <SDL_mixer.h>
#else
#define DIRECTSOUND_VERSION 0x1000
#include <mmsystem.h>
#include <dsound.h>
#endif
```

**Step 2: Verify modification**

Run: `type vs_projects\dxcommon\If\Sound.h | findstr SDL2`
Expected: SDL2 conditional found

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/Sound.h
git commit -m "Add SDL2 conditional compilation to Sound.h"
```

---

## Task 9: Modify Input.h for SDL2

**Files:**
- Modify: `vs_projects/dxcommon/If/Input.h`

**Step 1: Add SDL2 conditional includes to Input.h**

Replace the DirectInput include section (lines 10-14) with:

```cpp
#ifdef TOPSPEED_USE_SDL2
#include <DxCommon/If/SDL2Compat.h>
#include <SDL.h>
#else
// Microsoft apparently didn't define this, so let's do it ourselves
#define DIRECTINPUT_VERSION   0x0800
#include <dinput.h>
#endif
```

**Step 2: Verify modification**

Run: `type vs_projects\dxcommon\If\Input.h | findstr SDL2`
Expected: SDL2 conditional found

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/Input.h
git commit -m "Add SDL2 conditional compilation to Input.h"
```

---

## Task 10: Modify Network.h for SDL2/Multiplayer Disable

**Files:**
- Modify: `vs_projects/dxcommon/If/Network.h`

**Step 1: Add conditional compilation to Network.h**

Replace the `#include <dplay8.h>` line (line 11) with:

```cpp
#if defined(TOPSPEED_USE_SDL2) || defined(TOPSPEED_DISABLE_MULTIPLAYER)
#include <DxCommon/If/SDL2Compat.h>
#else
#include <dplay8.h>
#endif
```

**Step 2: Verify modification**

Run: `type vs_projects\dxcommon\If\Network.h | findstr DISABLE_MULTIPLAYER`
Expected: Conditional found

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/Network.h
git commit -m "Add multiplayer disable conditional to Network.h"
```

---

## Task 11: Modify Application.h for SDL2

**Files:**
- Modify: `vs_projects/dxcommon/If/Application.h`

**Step 1: Add SDL2 conditional includes to Application.h**

Replace the `#include <D3DX8.h>` line (line 13) with:

```cpp
#ifdef TOPSPEED_USE_SDL2
#include <DxCommon/If/SDL2Compat.h>
#else
#include <D3DX8.h>
#endif
```

**Step 2: Verify modification**

Run: `type vs_projects\dxcommon\If\Application.h | findstr SDL2`
Expected: SDL2 conditional found

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/Application.h
git commit -m "Add SDL2 conditional compilation to Application.h"
```

---

## Task 12: Create SDL2 Sound Implementation

**Files:**
- Create: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Create SDL2 Sound implementation**

```cpp
/**
 * DXCommon library - SDL2 Sound Implementation
 * Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
 * SDL2 port 2025
 */
#ifdef TOPSPEED_USE_SDL2

#include <DxCommon/If/Sound.h>
#include <Common/If/Tracer.h>
#include <cstring>
#include <cmath>

extern Tracer dxTracer;

namespace DirectX
{

//-----------------------------------------------------------------------------
// SoundManager Implementation
//-----------------------------------------------------------------------------

static bool g_sdlAudioInitialized = false;

SoundManager::SoundManager(::Window::Handle hwnd, UInt nChannels, UInt frequency, UInt bitrate)
    : m_directSound(nullptr)
    , m_created(false)
    , m_playInSoftware(false)
    , m_reverseStereo(false)
    , m_3dAlgorithm(AlgoDefault)
{
    if (!g_sdlAudioInitialized)
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
        {
            dxTracer.trace("SDL_InitSubSystem(AUDIO) failed: %s", SDL_GetError());
            return;
        }

        int mixerFlags = MIX_INIT_OGG;
        if ((Mix_Init(mixerFlags) & mixerFlags) != mixerFlags)
        {
            dxTracer.trace("Mix_Init failed: %s", Mix_GetError());
            return;
        }

        // Open audio device
        // frequency=22050 or 44100, format=AUDIO_S16SYS, channels=2, chunksize=1024
        if (Mix_OpenAudio(frequency, AUDIO_S16SYS, nChannels, 1024) < 0)
        {
            dxTracer.trace("Mix_OpenAudio failed: %s", Mix_GetError());
            return;
        }

        // Allocate mixing channels (default 8, but game may need more)
        Mix_AllocateChannels(32);

        g_sdlAudioInitialized = true;
    }
    m_created = true;
    dxTracer.trace("SoundManager: SDL2_mixer initialized (freq=%d, channels=%d)", frequency, nChannels);
}

SoundManager::~SoundManager()
{
    if (g_sdlAudioInitialized)
    {
        Mix_CloseAudio();
        Mix_Quit();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        g_sdlAudioInitialized = false;
    }
}

Sound* SoundManager::create(Int resource, Boolean enable3d, UInt nBuffers)
{
    // Resource-based loading not implemented for SDL2
    dxTracer.trace("SoundManager::create(resource) - not implemented");
    return nullptr;
}

Sound* SoundManager::create(Char* filename, Boolean enable3d, UInt nBuffers)
{
    Mix_Chunk* chunk = Mix_LoadWAV(filename);
    if (!chunk)
    {
        dxTracer.trace("Failed to load sound: %s - %s", filename, Mix_GetError());
        return nullptr;
    }

    // Create a Sound object wrapping the Mix_Chunk
    Sound* sound = new Sound(chunk, enable3d);
    sound->reverseStereo(m_reverseStereo);
    dxTracer.trace("Loaded sound: %s", filename);
    return sound;
}

Sound* SoundManager::create(DSBUFFERDESC& bufferDesc, Boolean enable3d, UInt nBuffers)
{
    // Buffer-based creation not implemented for SDL2
    dxTracer.trace("SoundManager::create(bufferDesc) - not implemented");
    return nullptr;
}

#ifdef _USE_VORBIS_
Sound* SoundManager::createVorbis(Char* filename, Boolean enable3d, UInt nBuffers)
{
    // SDL2_mixer handles OGG files natively via Mix_LoadWAV
    Mix_Chunk* chunk = Mix_LoadWAV(filename);
    if (!chunk)
    {
        dxTracer.trace("Failed to load vorbis sound: %s - %s", filename, Mix_GetError());
        return nullptr;
    }

    Sound* sound = new Sound(chunk, enable3d);
    sound->reverseStereo(m_reverseStereo);
    dxTracer.trace("Loaded vorbis sound: %s", filename);
    return sound;
}
#endif

Int SoundManager::bufferFormat(UInt nChannels, UInt frequency, UInt bitrate)
{
    // SDL2_mixer format is set at init time
    return dxSuccess;
}

Int SoundManager::listener3DInterface(LPDIRECTSOUND3DLISTENER* listener)
{
    // 3D audio stub
    *listener = nullptr;
    return dxSuccess;
}

//-----------------------------------------------------------------------------
// Sound Implementation
//-----------------------------------------------------------------------------

// Internal SDL2-specific data stored in unused DirectX pointers
struct SDL2SoundData
{
    Mix_Chunk* chunk;
    int channel;
    int volume;      // 0-128 for SDL_mixer
    int pan;         // -100 to +100
    bool is3d;
    float posX, posY, posZ;
};

// Helper to get SDL2 data from Sound
static SDL2SoundData* GetSDL2Data(Sound* sound)
{
    // We store SDL2SoundData pointer in m_buffer[0]
    return reinterpret_cast<SDL2SoundData*>(sound->buffer()[0]);
}

Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers, WaveFile* waveFile)
    : m_buffer(nullptr)
    , m_bufferSize(bufferSize)
    , m_nBuffers(nBuffers)
    , m_waveFile(waveFile)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    // This constructor is for DirectX compatibility, not used with SDL2
}

Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers, LPWAVEFORMATEX waveFormat)
    : m_buffer(nullptr)
    , m_bufferSize(bufferSize)
    , m_nBuffers(nBuffers)
    , m_waveFile(nullptr)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    // This constructor is for DirectX compatibility, not used with SDL2
}

// SDL2-specific constructor
Sound::Sound(Mix_Chunk* chunk, bool is3d)
    : m_buffer(new LPDIRECTSOUNDBUFFER[1])
    , m_bufferSize(0)
    , m_nBuffers(1)
    , m_waveFile(nullptr)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    SDL2SoundData* data = new SDL2SoundData();
    data->chunk = chunk;
    data->channel = -1;
    data->volume = MIX_MAX_VOLUME;
    data->pan = 0;
    data->is3d = is3d;
    data->posX = data->posY = data->posZ = 0.0f;

    m_buffer[0] = reinterpret_cast<LPDIRECTSOUNDBUFFER>(data);

    // Calculate length in seconds
    if (chunk && chunk->alen > 0)
    {
        // Approximate: assuming 44100Hz, 16-bit stereo
        m_length = static_cast<float>(chunk->alen) / (44100.0f * 2 * 2);
    }
}

#ifdef _USE_VORBIS_
Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers,
             OggVorbis_File* vorbisFile, UShort bitsPerSample, UInt avgBytesPerSec)
    : m_buffer(nullptr)
    , m_bufferSize(bufferSize)
    , m_nBuffers(nBuffers)
    , m_waveFile(nullptr)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    // Vorbis handled natively by SDL2_mixer, this constructor not used
}
#endif

Sound::~Sound()
{
    if (m_buffer)
    {
        SDL2SoundData* data = GetSDL2Data(this);
        if (data)
        {
            if (data->channel >= 0)
            {
                Mix_HaltChannel(data->channel);
            }
            if (data->chunk)
            {
                Mix_FreeChunk(data->chunk);
            }
            delete data;
        }
        delete[] m_buffer;
    }
}

Int Sound::play(UInt priority, Boolean looped)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || !data->chunk)
        return dxFailed;

    int loops = looped ? -1 : 0;
    data->channel = Mix_PlayChannel(-1, data->chunk, loops);

    if (data->channel < 0)
    {
        dxTracer.trace("Mix_PlayChannel failed: %s", Mix_GetError());
        return dxFailed;
    }

    // Apply current volume and pan
    Mix_Volume(data->channel, data->volume);

    // Apply pan (-100 to +100 -> SDL2 panning)
    int left = 255, right = 255;
    if (data->pan < 0)
    {
        right = 255 + (data->pan * 255 / 100);
    }
    else if (data->pan > 0)
    {
        left = 255 - (data->pan * 255 / 100);
    }
    if (m_reverseStereo < 0)
    {
        int tmp = left;
        left = right;
        right = tmp;
    }
    Mix_SetPanning(data->channel, left, right);

    return dxSuccess;
}

Int Sound::stop()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return dxSuccess;

    Mix_HaltChannel(data->channel);
    data->channel = -1;
    return dxSuccess;
}

Int Sound::reset()
{
    stop();
    return dxSuccess;
}

Boolean Sound::playing()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return false;

    return Mix_Playing(data->channel) != 0;
}

void Sound::pan(Int value)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return;

    // Clamp to -100..+100
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    data->pan = value;

    if (data->channel >= 0)
    {
        int left = 255, right = 255;
        if (value < 0)
        {
            right = 255 + (value * 255 / 100);
        }
        else if (value > 0)
        {
            left = 255 - (value * 255 / 100);
        }
        if (m_reverseStereo < 0)
        {
            int tmp = left;
            left = right;
            right = tmp;
        }
        Mix_SetPanning(data->channel, left, right);
    }
}

void Sound::frequency(Int value)
{
    // SDL2_mixer does not support frequency/pitch modification
    // This is a known limitation - noted in design document
    // Future: implement with SDL2 audio callback and pitch shifting
    dxTracer.trace("Sound::frequency(%d) - not implemented in SDL2", value);
}

Int Sound::frequency()
{
    // Return default frequency
    return 44100;
}

void Sound::volume(Int value)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return;

    // Convert 0-100 to 0-128
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    data->volume = (value * MIX_MAX_VOLUME) / 100;

    if (data->channel >= 0)
    {
        Mix_Volume(data->channel, data->volume);
    }
}

Int Sound::volume()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return 0;

    return (data->volume * 100) / MIX_MAX_VOLUME;
}

Int Sound::initializeBuffer3D(UInt index)
{
    // 3D audio initialization stub
    return dxSuccess;
}

void Sound::position(Vector3 pos)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || !data->is3d)
        return;

    data->posX = pos.x;
    data->posY = pos.y;
    data->posZ = pos.z;

    // Simple 3D audio: pan based on X position, volume based on distance
    if (data->channel >= 0)
    {
        // Pan: map X position to -100..+100 (assuming reasonable range)
        int panValue = static_cast<int>(pos.x * 10.0f);
        if (panValue < -100) panValue = -100;
        if (panValue > 100) panValue = 100;

        int left = 255, right = 255;
        if (panValue < 0)
        {
            right = 255 + (panValue * 255 / 100);
        }
        else if (panValue > 0)
        {
            left = 255 - (panValue * 255 / 100);
        }
        Mix_SetPanning(data->channel, left, right);

        // Distance attenuation
        float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
        int vol = data->volume;
        if (dist > 1.0f)
        {
            vol = static_cast<int>(data->volume / dist);
            if (vol < 0) vol = 0;
        }
        Mix_Volume(data->channel, vol);
    }
}

Int Sound::fillBufferWithSound(LPDIRECTSOUNDBUFFER buffer)
{
    return dxSuccess;
}

Int Sound::fillBufferWithSilence(LPDIRECTSOUNDBUFFER buffer, LPWAVEFORMATEX waveFormat)
{
    return dxSuccess;
}

#ifdef _USE_VORBIS_
Int Sound::fillBufferWithSound(LPDIRECTSOUNDBUFFER buffer, OggVorbis_File* vorbisFile, UShort bitsPerSample)
{
    return dxSuccess;
}
#endif

LPDIRECTSOUNDBUFFER Sound::getFreeBuffer()
{
    return nullptr;
}

WAVEFORMATEX* Sound::waveFormat()
{
    return nullptr;
}

UInt Sound::copyBuffer(LPDIRECTSOUNDBUFFER* buffer, UInt bufferOffset, UInt bufferSize)
{
    return 0;
}

UInt Sound::insertSilence(UInt bufferOffset, UInt bufferSize)
{
    return 0;
}

Int Sound::restoreBuffer(LPDIRECTSOUNDBUFFER buffer, Boolean* wasRestored)
{
    if (wasRestored) *wasRestored = false;
    return dxSuccess;
}

//-----------------------------------------------------------------------------
// WaveFile Implementation (minimal stub for SDL2)
//-----------------------------------------------------------------------------

WaveFile::WaveFile()
    : m_waveFormat(nullptr)
    , m_mmioHandle(nullptr)
    , m_size(0)
    , m_flags(0)
    , m_fromMemory(false)
    , m_data(nullptr)
    , m_dataCurrent(nullptr)
    , m_dataSize(0)
    , m_buffer(nullptr)
{
}

WaveFile::~WaveFile()
{
    close();
}

Int WaveFile::open(Char* filename, WAVEFORMATEX* format, UInt flags)
{
    // SDL2_mixer handles file loading directly, WaveFile not needed
    return dxFailed;
}

Int WaveFile::openFromMemory(UByte* buffer, UInt bufferSize, WAVEFORMATEX* format, UInt flags)
{
    return dxFailed;
}

Int WaveFile::close()
{
    return dxSuccess;
}

Int WaveFile::read(UByte* pBuffer, UInt dwSizeToRead, UInt* pdwSizeRead)
{
    if (pdwSizeRead) *pdwSizeRead = 0;
    return dxFailed;
}

Int WaveFile::write(UInt nSizeToWrite, UByte* pbData, UInt* pnSizeWrote)
{
    if (pnSizeWrote) *pnSizeWrote = 0;
    return dxFailed;
}

Int WaveFile::resetFile()
{
    return dxSuccess;
}

Int WaveFile::readMmio()
{
    return dxFailed;
}

Int WaveFile::writeMmio(WAVEFORMATEX* pwfxDest)
{
    return dxFailed;
}

//-----------------------------------------------------------------------------
// Listener3D Implementation (stub)
//-----------------------------------------------------------------------------

Listener3D::Listener3D(SoundManager* soundManager)
    : m_ds3DListener(nullptr)
{
}

Listener3D::~Listener3D()
{
}

void Listener3D::parameters(Float dopplerFactor, Float rolloffFactor)
{
    // Stub - SDL2_mixer doesn't have built-in 3D listener
}

void Listener3D::position(Vector3 pos)
{
    // Stub
}

void Listener3D::orientation(Float dir)
{
    // Stub
}

void Listener3D::positionAndOrientation(Vector3 pos, Float dir)
{
    // Stub
}

} // namespace DirectX

#endif // TOPSPEED_USE_SDL2
```

**Step 2: Verify file created**

Run: `type vs_projects\dxcommon\Src\Sound_SDL2.cpp | findstr "SDL2 Sound Implementation"`
Expected: Found

**Step 3: Update DxCommon CMakeLists.txt to include Sound_SDL2.cpp**

Add `Src/Sound_SDL2.cpp` to the source list.

**Step 4: Commit**

```bash
git add vs_projects/dxcommon/Src/Sound_SDL2.cpp vs_projects/dxcommon/CMakeLists.txt
git commit -m "Add SDL2 Sound implementation"
```

---

## Task 13: Create SDL2 Input Implementation

**Files:**
- Create: `vs_projects/dxcommon/Src/Input_SDL2.cpp`

**Step 1: Create SDL2 Input implementation**

```cpp
/**
 * DXCommon library - SDL2 Input Implementation
 * Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
 * SDL2 port 2025
 */
#ifdef TOPSPEED_USE_SDL2

#include <DxCommon/If/Input.h>
#include <Common/If/Tracer.h>
#include <SDL.h>
#include <cstring>

extern Tracer dxTracer;

namespace DirectX
{

static bool g_sdlInputInitialized = false;
static SDL_Joystick* g_joystick = nullptr;
static SDL_Haptic* g_haptic = nullptr;

//-----------------------------------------------------------------------------
// Input Base Class
//-----------------------------------------------------------------------------

// Base implementation is in header

//-----------------------------------------------------------------------------
// Keyboard Implementation
//-----------------------------------------------------------------------------

Keyboard::Keyboard(LPDIRECTINPUTDEVICE8 device)
    : m_device(device)
    , m_available(0)
{
    std::memset(m_keys, 0, sizeof(m_keys));
    std::memset(&m_state, 0, sizeof(m_state));
}

Keyboard::~Keyboard()
{
}

Int Keyboard::update()
{
    SDL_PumpEvents();

    const Uint8* sdlKeys = SDL_GetKeyboardState(nullptr);

    // Map SDL scancodes to DirectInput key codes
    // Common keys used in racing games:
    std::memset(m_state.keys, 0, sizeof(m_state.keys));

    // Arrow keys
    m_state.keys[DIK_UP] = sdlKeys[SDL_SCANCODE_UP];
    m_state.keys[DIK_DOWN] = sdlKeys[SDL_SCANCODE_DOWN];
    m_state.keys[DIK_LEFT] = sdlKeys[SDL_SCANCODE_LEFT];
    m_state.keys[DIK_RIGHT] = sdlKeys[SDL_SCANCODE_RIGHT];

    // Common game keys
    m_state.keys[DIK_SPACE] = sdlKeys[SDL_SCANCODE_SPACE];
    m_state.keys[DIK_RETURN] = sdlKeys[SDL_SCANCODE_RETURN];
    m_state.keys[DIK_ESCAPE] = sdlKeys[SDL_SCANCODE_ESCAPE];
    m_state.keys[DIK_TAB] = sdlKeys[SDL_SCANCODE_TAB];
    m_state.keys[DIK_BACK] = sdlKeys[SDL_SCANCODE_BACKSPACE];

    // Letter keys A-Z
    for (int i = 0; i < 26; i++)
    {
        m_state.keys[DIK_A + i] = sdlKeys[SDL_SCANCODE_A + i];
    }

    // Number keys 0-9
    m_state.keys[DIK_0] = sdlKeys[SDL_SCANCODE_0];
    m_state.keys[DIK_1] = sdlKeys[SDL_SCANCODE_1];
    m_state.keys[DIK_2] = sdlKeys[SDL_SCANCODE_2];
    m_state.keys[DIK_3] = sdlKeys[SDL_SCANCODE_3];
    m_state.keys[DIK_4] = sdlKeys[SDL_SCANCODE_4];
    m_state.keys[DIK_5] = sdlKeys[SDL_SCANCODE_5];
    m_state.keys[DIK_6] = sdlKeys[SDL_SCANCODE_6];
    m_state.keys[DIK_7] = sdlKeys[SDL_SCANCODE_7];
    m_state.keys[DIK_8] = sdlKeys[SDL_SCANCODE_8];
    m_state.keys[DIK_9] = sdlKeys[SDL_SCANCODE_9];

    // Function keys F1-F12
    for (int i = 0; i < 12; i++)
    {
        m_state.keys[DIK_F1 + i] = sdlKeys[SDL_SCANCODE_F1 + i];
    }

    // Modifier keys
    m_state.keys[DIK_LSHIFT] = sdlKeys[SDL_SCANCODE_LSHIFT];
    m_state.keys[DIK_RSHIFT] = sdlKeys[SDL_SCANCODE_RSHIFT];
    m_state.keys[DIK_LCONTROL] = sdlKeys[SDL_SCANCODE_LCTRL];
    m_state.keys[DIK_RCONTROL] = sdlKeys[SDL_SCANCODE_RCTRL];
    m_state.keys[DIK_LALT] = sdlKeys[SDL_SCANCODE_LALT];
    m_state.keys[DIK_RALT] = sdlKeys[SDL_SCANCODE_RALT];

    // Numpad
    m_state.keys[DIK_NUMPAD0] = sdlKeys[SDL_SCANCODE_KP_0];
    m_state.keys[DIK_NUMPAD1] = sdlKeys[SDL_SCANCODE_KP_1];
    m_state.keys[DIK_NUMPAD2] = sdlKeys[SDL_SCANCODE_KP_2];
    m_state.keys[DIK_NUMPAD3] = sdlKeys[SDL_SCANCODE_KP_3];
    m_state.keys[DIK_NUMPAD4] = sdlKeys[SDL_SCANCODE_KP_4];
    m_state.keys[DIK_NUMPAD5] = sdlKeys[SDL_SCANCODE_KP_5];
    m_state.keys[DIK_NUMPAD6] = sdlKeys[SDL_SCANCODE_KP_6];
    m_state.keys[DIK_NUMPAD7] = sdlKeys[SDL_SCANCODE_KP_7];
    m_state.keys[DIK_NUMPAD8] = sdlKeys[SDL_SCANCODE_KP_8];
    m_state.keys[DIK_NUMPAD9] = sdlKeys[SDL_SCANCODE_KP_9];

    // Copy to m_keys for compatibility
    std::memcpy(m_keys, m_state.keys, sizeof(m_keys));

    return dxSuccess;
}

Int Keyboard::dataFormat()
{
    return dxSuccess;
}

Int Keyboard::cooperativeLevel(::Window::Handle handle, UInt flags)
{
    return dxSuccess;
}

//-----------------------------------------------------------------------------
// Joystick Implementation
//-----------------------------------------------------------------------------

Joystick::Joystick(LPDIRECTINPUTDEVICE8 device)
    : m_device(device)
    , m_available(0)
    , m_forceFeedbackCapable(false)
{
    std::memset(&m_state, 0, sizeof(m_state));
}

Joystick::~Joystick()
{
}

Int Joystick::update()
{
    if (!g_joystick)
        return dxFailed;

    SDL_JoystickUpdate();

    // Read axes (normalized to -1000..+1000 like DirectInput)
    int numAxes = SDL_JoystickNumAxes(g_joystick);

    if (numAxes > 0) m_state.x = SDL_JoystickGetAxis(g_joystick, 0) * 1000 / 32767;
    if (numAxes > 1) m_state.y = SDL_JoystickGetAxis(g_joystick, 1) * 1000 / 32767;
    if (numAxes > 2) m_state.z = SDL_JoystickGetAxis(g_joystick, 2) * 1000 / 32767;
    if (numAxes > 3) m_state.rx = SDL_JoystickGetAxis(g_joystick, 3) * 1000 / 32767;
    if (numAxes > 4) m_state.ry = SDL_JoystickGetAxis(g_joystick, 4) * 1000 / 32767;
    if (numAxes > 5) m_state.rz = SDL_JoystickGetAxis(g_joystick, 5) * 1000 / 32767;

    // Read buttons
    int numButtons = SDL_JoystickNumButtons(g_joystick);
    m_state.b1 = numButtons > 0 ? SDL_JoystickGetButton(g_joystick, 0) : false;
    m_state.b2 = numButtons > 1 ? SDL_JoystickGetButton(g_joystick, 1) : false;
    m_state.b3 = numButtons > 2 ? SDL_JoystickGetButton(g_joystick, 2) : false;
    m_state.b4 = numButtons > 3 ? SDL_JoystickGetButton(g_joystick, 3) : false;
    m_state.b5 = numButtons > 4 ? SDL_JoystickGetButton(g_joystick, 4) : false;
    m_state.b6 = numButtons > 5 ? SDL_JoystickGetButton(g_joystick, 5) : false;
    m_state.b7 = numButtons > 6 ? SDL_JoystickGetButton(g_joystick, 6) : false;
    m_state.b8 = numButtons > 7 ? SDL_JoystickGetButton(g_joystick, 7) : false;
    m_state.b9 = numButtons > 8 ? SDL_JoystickGetButton(g_joystick, 8) : false;
    m_state.b10 = numButtons > 9 ? SDL_JoystickGetButton(g_joystick, 9) : false;
    m_state.b11 = numButtons > 10 ? SDL_JoystickGetButton(g_joystick, 10) : false;
    m_state.b12 = numButtons > 11 ? SDL_JoystickGetButton(g_joystick, 11) : false;
    m_state.b13 = numButtons > 12 ? SDL_JoystickGetButton(g_joystick, 12) : false;
    m_state.b14 = numButtons > 13 ? SDL_JoystickGetButton(g_joystick, 13) : false;
    m_state.b15 = numButtons > 14 ? SDL_JoystickGetButton(g_joystick, 14) : false;
    m_state.b16 = numButtons > 15 ? SDL_JoystickGetButton(g_joystick, 15) : false;

    // Read POV/hat
    int numHats = SDL_JoystickNumHats(g_joystick);
    if (numHats > 0)
    {
        Uint8 hat = SDL_JoystickGetHat(g_joystick, 0);
        m_state.pov1 = (hat & SDL_HAT_UP) != 0;
        m_state.pov2 = (hat & SDL_HAT_RIGHT) != 0;
        m_state.pov3 = (hat & SDL_HAT_DOWN) != 0;
        m_state.pov4 = (hat & SDL_HAT_LEFT) != 0;
    }

    return dxSuccess;
}

Int Joystick::setRange(UInt axis, Int min, Int max)
{
    // SDL2 joystick axes are always -32768 to 32767
    // We scale in update() to match the expected range
    return dxSuccess;
}

Int Joystick::dataFormat()
{
    return dxSuccess;
}

Int Joystick::cooperativeLevel(::Window::Handle handle)
{
    return dxSuccess;
}

void Joystick::autocenter(Boolean b)
{
    // Force feedback autocenter - stub
}

//-----------------------------------------------------------------------------
// InputManager Implementation
//-----------------------------------------------------------------------------

InputManager::InputManager()
    : m_initialized(false)
    , m_directInput(nullptr)
    , m_joystick(nullptr)
    , m_keyboard(nullptr)
    , m_handle(nullptr)
{
}

InputManager::~InputManager()
{
    finalize();
}

Int InputManager::initialize(Window::Handle handle)
{
    if (m_initialized)
        return dxSuccess;

    m_handle = handle;

    if (!g_sdlInputInitialized)
    {
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) < 0)
        {
            dxTracer.trace("SDL_InitSubSystem(JOYSTICK|HAPTIC) failed: %s", SDL_GetError());
            // Continue without joystick
        }
        g_sdlInputInitialized = true;
    }

    // Create keyboard
    m_keyboard = new Keyboard(nullptr);
    dxTracer.trace("InputManager: Keyboard initialized");

    // Try to open first joystick
    int numJoysticks = SDL_NumJoysticks();
    dxTracer.trace("InputManager: Found %d joystick(s)", numJoysticks);

    if (numJoysticks > 0)
    {
        g_joystick = SDL_JoystickOpen(0);
        if (g_joystick)
        {
            m_joystick = new Joystick(nullptr);

            const char* name = SDL_JoystickName(g_joystick);
            dxTracer.trace("InputManager: Opened joystick: %s", name ? name : "Unknown");

            // Check for force feedback
            if (SDL_JoystickIsHaptic(g_joystick))
            {
                g_haptic = SDL_HapticOpenFromJoystick(g_joystick);
                if (g_haptic)
                {
                    m_joystick->FFcapable(true);
                    dxTracer.trace("InputManager: Haptic feedback available");
                }
            }
        }
    }

    m_initialized = true;
    return dxSuccess;
}

void InputManager::finalize()
{
    if (!m_initialized)
        return;

    delete m_keyboard;
    m_keyboard = nullptr;

    delete m_joystick;
    m_joystick = nullptr;

    if (g_haptic)
    {
        SDL_HapticClose(g_haptic);
        g_haptic = nullptr;
    }

    if (g_joystick)
    {
        SDL_JoystickClose(g_joystick);
        g_joystick = nullptr;
    }

    if (g_sdlInputInitialized)
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
        g_sdlInputInitialized = false;
    }

    m_initialized = false;
}

Int InputManager::update()
{
    // Process SDL events to keep keyboard state current
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            // Could signal quit to application
        }
    }

    if (m_keyboard)
        m_keyboard->update();

    if (m_joystick)
        m_joystick->update();

    return dxSuccess;
}

const Input::State InputManager::state()
{
    Input::State combined;
    std::memset(&combined, 0, sizeof(combined));

    if (m_keyboard)
    {
        std::memcpy(combined.keys, m_keyboard->state().keys, sizeof(combined.keys));
    }

    if (m_joystick)
    {
        const Input::State& js = m_joystick->state();
        combined.x = js.x;
        combined.y = js.y;
        combined.z = js.z;
        combined.rx = js.rx;
        combined.ry = js.ry;
        combined.rz = js.rz;
        combined.slider1 = js.slider1;
        combined.slider2 = js.slider2;
        combined.b1 = js.b1;
        combined.b2 = js.b2;
        combined.b3 = js.b3;
        combined.b4 = js.b4;
        combined.b5 = js.b5;
        combined.b6 = js.b6;
        combined.b7 = js.b7;
        combined.b8 = js.b8;
        combined.b9 = js.b9;
        combined.b10 = js.b10;
        combined.b11 = js.b11;
        combined.b12 = js.b12;
        combined.b13 = js.b13;
        combined.b14 = js.b14;
        combined.b15 = js.b15;
        combined.b16 = js.b16;
        combined.pov1 = js.pov1;
        combined.pov2 = js.pov2;
        combined.pov3 = js.pov3;
        combined.pov4 = js.pov4;
        combined.pov5 = js.pov5;
        combined.pov6 = js.pov6;
        combined.pov7 = js.pov7;
        combined.pov8 = js.pov8;
    }

    return combined;
}

// Stub callbacks (not used with SDL2)
BOOL CALLBACK InputManager::enumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    return FALSE;
}

BOOL CALLBACK InputManager::enumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
    return FALSE;
}

BOOL CALLBACK InputManager::enumKeyboardsCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    return FALSE;
}

//-----------------------------------------------------------------------------
// ForceFeedbackEffect Implementation
//-----------------------------------------------------------------------------

ForceFeedbackEffect::ForceFeedbackEffect(Joystick* joystick)
    : m_joystick(joystick)
{
}

ForceFeedbackEffect::ForceFeedbackEffect(Joystick* joystick, char* filename)
    : m_joystick(joystick)
{
    // Loading .ffe files not implemented for SDL2
    // SDL2 haptic uses different effect definitions
    dxTracer.trace("ForceFeedbackEffect: .ffe file loading not implemented for SDL2");
}

ForceFeedbackEffect::~ForceFeedbackEffect()
{
    stop();
}

void ForceFeedbackEffect::play()
{
    if (!g_haptic)
        return;

    // Simple rumble effect as placeholder
    if (SDL_HapticRumbleSupported(g_haptic))
    {
        SDL_HapticRumbleInit(g_haptic);
        SDL_HapticRumblePlay(g_haptic, 0.5f, 200);
    }
}

void ForceFeedbackEffect::stop()
{
    if (!g_haptic)
        return;

    SDL_HapticRumbleStop(g_haptic);
}

void ForceFeedbackEffect::gain(UInt gain)
{
    // Gain control stub
}

void ForceFeedbackEffect::addFileEffect(LPCDIFILEEFFECT fileEffect)
{
    // Not implemented for SDL2
}

BOOL CALLBACK ForceFeedbackEffect::EnumAndCreateEffectsCallback(LPCDIFILEEFFECT effect, VOID* classRef)
{
    return FALSE;
}

} // namespace DirectX

// DirectInput key code definitions for SDL2 compatibility
#ifndef DIK_ESCAPE
#define DIK_ESCAPE          0x01
#define DIK_1               0x02
#define DIK_2               0x03
#define DIK_3               0x04
#define DIK_4               0x05
#define DIK_5               0x06
#define DIK_6               0x07
#define DIK_7               0x08
#define DIK_8               0x09
#define DIK_9               0x0A
#define DIK_0               0x0B
#define DIK_BACK            0x0E
#define DIK_TAB             0x0F
#define DIK_Q               0x10
#define DIK_W               0x11
#define DIK_E               0x12
#define DIK_R               0x13
#define DIK_T               0x14
#define DIK_Y               0x15
#define DIK_U               0x16
#define DIK_I               0x17
#define DIK_O               0x18
#define DIK_P               0x19
#define DIK_RETURN          0x1C
#define DIK_LCONTROL        0x1D
#define DIK_A               0x1E
#define DIK_S               0x1F
#define DIK_D               0x20
#define DIK_F               0x21
#define DIK_G               0x22
#define DIK_H               0x23
#define DIK_J               0x24
#define DIK_K               0x25
#define DIK_L               0x26
#define DIK_LSHIFT          0x2A
#define DIK_Z               0x2C
#define DIK_X               0x2D
#define DIK_C               0x2E
#define DIK_V               0x2F
#define DIK_B               0x30
#define DIK_N               0x31
#define DIK_M               0x32
#define DIK_RSHIFT          0x36
#define DIK_LALT            0x38
#define DIK_SPACE           0x39
#define DIK_F1              0x3B
#define DIK_F2              0x3C
#define DIK_F3              0x3D
#define DIK_F4              0x3E
#define DIK_F5              0x3F
#define DIK_F6              0x40
#define DIK_F7              0x41
#define DIK_F8              0x42
#define DIK_F9              0x43
#define DIK_F10             0x44
#define DIK_NUMPAD7         0x47
#define DIK_NUMPAD8         0x48
#define DIK_NUMPAD9         0x49
#define DIK_NUMPAD4         0x4B
#define DIK_NUMPAD5         0x4C
#define DIK_NUMPAD6         0x4D
#define DIK_NUMPAD1         0x4F
#define DIK_NUMPAD2         0x50
#define DIK_NUMPAD3         0x51
#define DIK_NUMPAD0         0x52
#define DIK_F11             0x57
#define DIK_F12             0x58
#define DIK_RCONTROL        0x9D
#define DIK_RALT            0xB8
#define DIK_UP              0xC8
#define DIK_LEFT            0xCB
#define DIK_RIGHT           0xCD
#define DIK_DOWN            0xD0
#endif

#endif // TOPSPEED_USE_SDL2
```

**Step 2: Verify file created**

Run: `type vs_projects\dxcommon\Src\Input_SDL2.cpp | findstr "SDL2 Input Implementation"`
Expected: Found

**Step 3: Update DxCommon CMakeLists.txt to include Input_SDL2.cpp**

Add `Src/Input_SDL2.cpp` to the source list.

**Step 4: Commit**

```bash
git add vs_projects/dxcommon/Src/Input_SDL2.cpp vs_projects/dxcommon/CMakeLists.txt
git commit -m "Add SDL2 Input implementation"
```

---

## Task 14: Create Stub Implementations for Graphics/Network

**Files:**
- Create: `vs_projects/dxcommon/Src/Stubs_SDL2.cpp`

**Step 1: Create stub implementations**

```cpp
/**
 * DXCommon library - SDL2 Stub Implementations
 * Stubs for graphics and network APIs when using SDL2
 */
#ifdef TOPSPEED_USE_SDL2

#include <DxCommon/If/Application.h>
#include <DxCommon/If/D3DFont.h>
#include <DxCommon/If/D3DSprite.h>
#include <DxCommon/If/Light.h>
#include <DxCommon/If/Line.h>
#include <DxCommon/If/Mesh.h>
#include <DxCommon/If/Network.h>
#include <DxCommon/If/Particle.h>
#include <DxCommon/If/Game.h>
#include <Common/If/Tracer.h>
#include <SDL.h>

extern Tracer dxTracer;

namespace DirectX
{

//-----------------------------------------------------------------------------
// Application Implementation (minimal SDL2 window)
//-----------------------------------------------------------------------------

static SDL_Window* g_window = nullptr;

Application::Application()
    : m_control3D(nullptr)
    , m_mainWindow(nullptr)
    , m_active(true)
    , m_fullscreen(false)
    , m_screenWidth(800)
    , m_screenHeight(600)
    , m_d3dDevice(nullptr)
    , m_font(nullptr)
    , m_elapsed(0)
    , m_showFPS(false)
    , m_backgroundColor(0)
{
}

Application::~Application()
{
    if (g_window)
    {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
}

Int Application::create(Window::Instance instance, Int icon)
{
    // Initialize SDL video if not already done
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        dxTracer.trace("SDL_Init(VIDEO) failed: %s", SDL_GetError());
        return dxFailed;
    }

    // Create a minimal window (game is audio-based, so window is just a placeholder)
    g_window = SDL_CreateWindow(
        "Top Speed 3",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_screenWidth, m_screenHeight,
        SDL_WINDOW_SHOWN
    );

    if (!g_window)
    {
        dxTracer.trace("SDL_CreateWindow failed: %s", SDL_GetError());
        return dxFailed;
    }

    m_mainWindow = (Window::Handle)g_window;
    dxTracer.trace("Application: SDL2 window created (%dx%d)", m_screenWidth, m_screenHeight);

    return dxSuccess;
}

Int Application::run()
{
    // Main loop handled elsewhere (MFC OnIdle)
    return dxSuccess;
}

Int Application::displayMode(Int width, Int height, Boolean fullscreen)
{
    m_screenWidth = width;
    m_screenHeight = height;
    m_fullscreen = fullscreen;

    if (g_window)
    {
        SDL_SetWindowSize(g_window, width, height);
        if (fullscreen)
        {
            SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else
        {
            SDL_SetWindowFullscreen(g_window, 0);
        }
    }

    return dxSuccess;
}

Int Application::initializeDisplay()
{
    return dxSuccess;
}

LRESULT Application::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Application::Control3D Implementation (stub)
//-----------------------------------------------------------------------------

Application::Control3D::Control3D(LPDIRECT3DDEVICE8 device, Int screenWidth, Int screenHeight)
    : m_d3dDevice(device)
    , m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
}

Application::Control3D::~Control3D()
{
}

void Application::Control3D::view(Float fromx, Float fromy, Float fromz,
                                   Float tox, Float toy, Float toz)
{
}

void Application::Control3D::view(Vector3 from, Vector3 to)
{
}

void Application::Control3D::projection(Float Near, Float Far)
{
}

void Application::Control3D::material(UInt color, Float alpha)
{
}

void Application::Control3D::ambient(UInt color)
{
}

//-----------------------------------------------------------------------------
// D3DFont Implementation (stub)
//-----------------------------------------------------------------------------

D3DFont::D3DFont(const TCHAR* strFontName, DWORD dwHeight, DWORD dwFlags)
{
}

D3DFont::~D3DFont()
{
}

HRESULT D3DFont::InitDeviceObjects(LPDIRECT3DDEVICE8 pd3dDevice)
{
    return S_OK;
}

HRESULT D3DFont::RestoreDeviceObjects()
{
    return S_OK;
}

HRESULT D3DFont::InvalidateDeviceObjects()
{
    return S_OK;
}

HRESULT D3DFont::DeleteDeviceObjects()
{
    return S_OK;
}

HRESULT D3DFont::DrawText(FLOAT x, FLOAT y, DWORD dwColor,
                          const TCHAR* strText, DWORD dwFlags)
{
    return S_OK;
}

//-----------------------------------------------------------------------------
// Network Implementation (stub - multiplayer disabled)
//-----------------------------------------------------------------------------

#ifdef TOPSPEED_DISABLE_MULTIPLAYER

Server::Server()
    : m_directPlayServer(nullptr)
    , m_directPlayAddress(nullptr)
    , m_iServer(nullptr)
    , m_started(false)
{
}

Server::~Server()
{
}

Int Server::startSession(Char* name, UInt port)
{
    dxTracer.trace("Server::startSession - multiplayer disabled");
    return dxFailed;
}

void Server::stopSession()
{
}

void Server::sendPacket(UInt to, void* buffer, UInt size, Boolean secure, UInt timeout)
{
}

HRESULT WINAPI Server::directPlayMessageHandler(void* pvUserContext, DWORD dwMessageId, void* pMsgBuffer)
{
    return S_OK;
}

HRESULT WINAPI Server::localMessageHandler(UInt messageId, void* msgBuffer)
{
    return S_OK;
}

Client::Client()
    : m_directPlayClient(nullptr)
    , m_iClient(nullptr)
    , m_directPlayEnumHandle(0)
    , m_nSessions(0)
{
}

Client::~Client()
{
}

Int Client::initialize()
{
    dxTracer.trace("Client::initialize - multiplayer disabled");
    return dxFailed;
}

Int Client::finalize()
{
    return dxSuccess;
}

Int Client::sendPacket(void* buffer, UInt size, Boolean secure, UInt timeout)
{
    return dxFailed;
}

Int Client::startSessionEnum(UInt port, const Char* ipaddress)
{
    return dxFailed;
}

Int Client::stopSessionEnum()
{
    return dxSuccess;
}

Int Client::session(UInt i, SessionInfo& info)
{
    return dxFailed;
}

Int Client::joinSession(UInt i)
{
    return dxFailed;
}

Int Client::joinSessionAt(UInt port, const Char* ipaddress)
{
    return dxFailed;
}

void Client::sessionEnumCallback(const DPN_APPLICATION_DESC* appDesc,
                                  IDirectPlay8Address* addressHost,
                                  IDirectPlay8Address* addressDevice)
{
}

HRESULT WINAPI Client::directPlayMessageHandler(void* pvUserContext, DWORD dwMessageId, void* pMsgBuffer)
{
    return S_OK;
}

HRESULT WINAPI Client::localMessageHandler(UInt messageId, void* msgBuffer)
{
    return S_OK;
}

#endif // TOPSPEED_DISABLE_MULTIPLAYER

} // namespace DirectX

#endif // TOPSPEED_USE_SDL2
```

**Step 2: Verify file created**

Run: `type vs_projects\dxcommon\Src\Stubs_SDL2.cpp | findstr "SDL2 Stub Implementations"`
Expected: Found

**Step 3: Update DxCommon CMakeLists.txt**

Add `Src/Stubs_SDL2.cpp` to the source list.

**Step 4: Commit**

```bash
git add vs_projects/dxcommon/Src/Stubs_SDL2.cpp vs_projects/dxcommon/CMakeLists.txt
git commit -m "Add SDL2 stub implementations for graphics and network"
```

---

## Task 15: Add DIK Key Definitions Header

**Files:**
- Create: `vs_projects/dxcommon/If/DIKCodes.h`

**Step 1: Create DirectInput key code definitions**

```cpp
/**
 * DirectInput Key Codes for SDL2 Compatibility
 * These match the original DirectInput scan codes
 */
#ifndef __DXCOMMON_DIKCODES_H__
#define __DXCOMMON_DIKCODES_H__

#ifdef TOPSPEED_USE_SDL2

#define DIK_ESCAPE          0x01
#define DIK_1               0x02
#define DIK_2               0x03
#define DIK_3               0x04
#define DIK_4               0x05
#define DIK_5               0x06
#define DIK_6               0x07
#define DIK_7               0x08
#define DIK_8               0x09
#define DIK_9               0x0A
#define DIK_0               0x0B
#define DIK_MINUS           0x0C
#define DIK_EQUALS          0x0D
#define DIK_BACK            0x0E
#define DIK_TAB             0x0F
#define DIK_Q               0x10
#define DIK_W               0x11
#define DIK_E               0x12
#define DIK_R               0x13
#define DIK_T               0x14
#define DIK_Y               0x15
#define DIK_U               0x16
#define DIK_I               0x17
#define DIK_O               0x18
#define DIK_P               0x19
#define DIK_LBRACKET        0x1A
#define DIK_RBRACKET        0x1B
#define DIK_RETURN          0x1C
#define DIK_LCONTROL        0x1D
#define DIK_A               0x1E
#define DIK_S               0x1F
#define DIK_D               0x20
#define DIK_F               0x21
#define DIK_G               0x22
#define DIK_H               0x23
#define DIK_J               0x24
#define DIK_K               0x25
#define DIK_L               0x26
#define DIK_SEMICOLON       0x27
#define DIK_APOSTROPHE      0x28
#define DIK_GRAVE           0x29
#define DIK_LSHIFT          0x2A
#define DIK_BACKSLASH       0x2B
#define DIK_Z               0x2C
#define DIK_X               0x2D
#define DIK_C               0x2E
#define DIK_V               0x2F
#define DIK_B               0x30
#define DIK_N               0x31
#define DIK_M               0x32
#define DIK_COMMA           0x33
#define DIK_PERIOD          0x34
#define DIK_SLASH           0x35
#define DIK_RSHIFT          0x36
#define DIK_MULTIPLY        0x37
#define DIK_LALT            0x38
#define DIK_SPACE           0x39
#define DIK_CAPITAL         0x3A
#define DIK_F1              0x3B
#define DIK_F2              0x3C
#define DIK_F3              0x3D
#define DIK_F4              0x3E
#define DIK_F5              0x3F
#define DIK_F6              0x40
#define DIK_F7              0x41
#define DIK_F8              0x42
#define DIK_F9              0x43
#define DIK_F10             0x44
#define DIK_NUMLOCK         0x45
#define DIK_SCROLL          0x46
#define DIK_NUMPAD7         0x47
#define DIK_NUMPAD8         0x48
#define DIK_NUMPAD9         0x49
#define DIK_SUBTRACT        0x4A
#define DIK_NUMPAD4         0x4B
#define DIK_NUMPAD5         0x4C
#define DIK_NUMPAD6         0x4D
#define DIK_ADD             0x4E
#define DIK_NUMPAD1         0x4F
#define DIK_NUMPAD2         0x50
#define DIK_NUMPAD3         0x51
#define DIK_NUMPAD0         0x52
#define DIK_DECIMAL         0x53
#define DIK_F11             0x57
#define DIK_F12             0x58
#define DIK_NUMPADENTER     0x9C
#define DIK_RCONTROL        0x9D
#define DIK_DIVIDE          0xB5
#define DIK_RALT            0xB8
#define DIK_HOME            0xC7
#define DIK_UP              0xC8
#define DIK_PRIOR           0xC9
#define DIK_LEFT            0xCB
#define DIK_RIGHT           0xCD
#define DIK_END             0xCF
#define DIK_DOWN            0xD0
#define DIK_NEXT            0xD1
#define DIK_INSERT          0xD2
#define DIK_DELETE          0xD3

#endif // TOPSPEED_USE_SDL2

#endif // __DXCOMMON_DIKCODES_H__
```

**Step 2: Update Input_SDL2.cpp to include DIKCodes.h**

Add at the top of Input_SDL2.cpp after the includes:
```cpp
#include <DxCommon/If/DIKCodes.h>
```

And remove the inline DIK definitions at the bottom of that file.

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/If/DIKCodes.h vs_projects/dxcommon/Src/Input_SDL2.cpp
git commit -m "Add DIK key code definitions header"
```

---

## Task 16: Create Build Documentation

**Files:**
- Create: `docs/BUILD.md`

**Step 1: Create build documentation**

```markdown
# Building Top Speed 3

## Prerequisites

### Visual Studio 2022
Install Visual Studio 2022 with the following workloads:
- Desktop development with C++
- MFC/ATL support (under Individual Components)

### vcpkg
1. Clone vcpkg:
   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. Set environment variable:
   ```bash
   setx VCPKG_ROOT "C:\path\to\vcpkg"
   ```

3. Integrate with Visual Studio:
   ```bash
   .\vcpkg integrate install
   ```

### CMake
CMake 3.21+ is required. Install via Visual Studio Installer or download from cmake.org.

## Building

### Using CMake Presets (Recommended)

```bash
# Configure Debug build
cmake --preset=vs2022-debug

# Build
cmake --build --preset=vs2022-debug

# Or open in Visual Studio
start build\vs2022-debug\TopSpeed.sln
```

### Manual CMake

```bash
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A Win32 ^
      -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
      ..
cmake --build . --config Debug
```

## Project Structure

```
TopSpeed/
├── CMakeLists.txt          # Root CMake configuration
├── CMakePresets.json       # Build presets for VS2022
├── vcpkg.json              # Dependency manifest
├── vs_projects/
│   ├── common/             # Platform utilities library
│   ├── dxcommon/           # SDL2 abstraction layer
│   └── topspeed/           # Game executable
├── docs/
│   ├── BUILD.md            # This file
│   └── plans/              # Design documents
└── installer/              # Inno Setup installer
```

## Dependencies

Managed automatically by vcpkg:
- SDL2 - Window management, input
- SDL2_mixer - Audio playback (with OGG/Vorbis support)

## Configuration Flags

| Flag | Description |
|------|-------------|
| TOPSPEED_USE_SDL2 | Use SDL2 instead of DirectX (default: ON) |
| TOPSPEED_DISABLE_MULTIPLAYER | Disable network multiplayer (default: ON) |

## Known Limitations

1. **Audio Pitch Control**: SDL2_mixer doesn't support real-time pitch/frequency changes. Engine sounds won't change pitch with RPM. Future work will implement this via SDL2 audio callbacks.

2. **Force Feedback**: Basic rumble effects work. Complex .ffe files from the original game are not fully supported.

3. **Multiplayer**: Disabled in this build. Original DirectPlay networking is not supported.

## Troubleshooting

### vcpkg not found
Ensure `VCPKG_ROOT` environment variable is set and restart your terminal/IDE.

### MFC headers not found
Install "C++ MFC for latest v143 build tools" via Visual Studio Installer.

### SDL2 not found
Run `vcpkg install sdl2:x86-windows sdl2-mixer:x86-windows` manually, then reconfigure.
```

**Step 2: Verify file created**

Run: `type docs\BUILD.md | findstr "Building Top Speed"`
Expected: Found

**Step 3: Commit**

```bash
git add docs/BUILD.md
git commit -m "Add build documentation"
```

---

## Task 17: Test CMake Configuration

**Step 1: Run CMake configure**

Run:
```bash
cmake --preset=vs2022-debug
```

Expected: Configuration completes (may have warnings but no errors about missing SDL2)

**Step 2: If errors, diagnose and fix**

Common issues:
- vcpkg not found: Check VCPKG_ROOT
- SDL2 not found: vcpkg will auto-install on configure

**Step 3: Commit any fixes**

```bash
git add -A
git commit -m "Fix CMake configuration issues"
```

---

## Task 18: Build and Fix Compilation Errors

**Step 1: Build the project**

Run:
```bash
cmake --build --preset=vs2022-debug
```

**Step 2: Fix compilation errors iteratively**

This will likely require:
- Adding missing includes
- Fixing type mismatches
- Adding stub methods

**Step 3: Commit fixes**

```bash
git add -A
git commit -m "Fix compilation errors for SDL2 build"
```

---

## Task 19: Verify Executable Runs

**Step 1: Run the built executable**

Run:
```bash
.\build\vs2022-debug\vs_projects\topspeed\Debug\TopSpeed.exe
```

Expected: Application launches (may not be fully functional yet)

**Step 2: Check for runtime errors**

Review TopSpeed.trc if tracing is enabled, or debug output.

**Step 3: Commit final integration**

```bash
git add -A
git commit -m "Complete SDL2 migration - audio and input functional"
```

---

## Summary

Total tasks: 19
Estimated complexity: Medium-High

Key commits:
1. Root CMake configuration
2. Library CMakeLists.txt files
3. SDL2 compatibility headers
4. SDL2 Sound implementation
5. SDL2 Input implementation
6. Stub implementations
7. Build documentation
8. Compilation fixes
9. Final integration
