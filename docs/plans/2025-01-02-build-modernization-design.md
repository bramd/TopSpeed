# Build Modernization Design

## Overview

Modernize Top Speed 3's build system from VS2008/DirectX8 to VS2022/SDL2 with CMake, preparing the codebase for eventual WASM compilation.

## Goals

1. Build with VS2022 using CMake-generated solutions
2. Replace DirectX dependencies with SDL2 (audio + input functional, graphics stubbed)
3. Disable multiplayer (DirectPlay) with conditional compilation
4. Manage dependencies via vcpkg
5. Document the build process for CI reproduction

## Non-Goals (This Milestone)

- Full graphics implementation
- Multiplayer functionality
- WASM compilation (future milestone)
- Game feature changes

## Architecture

### Current Structure

```
TopSpeed.exe
    ├── DxCommonStatic.lib (DirectX 8 wrapper)
    │       └── CommonStatic.lib (platform utilities)
    └── libogg, libvorbis, libvorbisfile
```

### New Structure

```
TopSpeed.exe
    ├── DxCommon.lib (SDL2 wrapper - renamed internally)
    │       └── Common.lib
    └── SDL2, SDL2_mixer (via vcpkg, handles vorbis internally)
```

## Dependencies

Managed via vcpkg:

| Package | Purpose |
|---------|---------|
| sdl2 | Window, input, graphics stub |
| sdl2-mixer | Audio playback (replaces DirectSound) |

Note: SDL2_mixer includes ogg/vorbis support, so separate libogg/libvorbis may not be needed.

## Implementation Approach

### 1. CMake Structure

```
TopSpeed/
├── CMakeLists.txt              # Root CMake file
├── CMakePresets.json           # VS2022 + vcpkg presets
├── vcpkg.json                  # Dependency manifest
├── vs_projects/
│   ├── common/
│   │   └── CMakeLists.txt      # Common static library
│   ├── dxcommon/
│   │   └── CMakeLists.txt      # DxCommon static library
│   └── topspeed/
│       └── CMakeLists.txt      # Main executable
└── docs/
    └── BUILD.md                # Build instructions
```

### 2. Conditional Compilation

Add preprocessor defines to disable DirectPlay:

```cpp
// Disable multiplayer
#define TOPSPEED_DISABLE_MULTIPLAYER 1

// Use SDL2 instead of DirectX
#define TOPSPEED_USE_SDL2 1
```

Affected files:
- `RaceServer.cpp` / `RaceClient.cpp` - Wrap in `#ifndef TOPSPEED_DISABLE_MULTIPLAYER`
- `LevelMultiPlayer.cpp` - Stub or disable
- `Network.cpp` (in DxCommon) - Replace DirectPlay with stubs

### 3. DxCommon SDL2 Adaptation

The DxCommon library wraps DirectX. We'll create SDL2 implementations:

| DxCommon Module | DirectX API | SDL2 Replacement |
|-----------------|-------------|------------------|
| Sound.cpp | DirectSound | SDL2_mixer |
| Input.cpp | DirectInput8 | SDL2 input |
| Game.cpp | D3D8 device | SDL2 window + stub renderer |
| Network.cpp | DirectPlay | Disabled (stubs) |
| D3DFont.cpp | D3DX8 | Stub (no-op) |
| D3DSprite.cpp | D3DX8 | Stub (no-op) |
| Mesh.cpp | D3DX8 | Stub (no-op) |

### 4. Sound System Mapping

DirectSound concepts to SDL2_mixer:

| DirectSound | SDL2_mixer |
|-------------|------------|
| IDirectSoundBuffer | Mix_Chunk |
| IDirectSoundBuffer::Play | Mix_PlayChannel |
| IDirectSoundBuffer::SetVolume | Mix_VolumeChunk |
| IDirectSoundBuffer::SetPan | Mix_SetPanning |
| IDirectSoundBuffer::SetFrequency | Mix_SetMusicPosition (limited) |
| 3D sound positioning | Mix_SetPosition |

Note: DirectSound frequency manipulation (for engine pitch) may need SDL2's lower-level audio API or a pitch-shifting library.

### 5. Input System Mapping

| DirectInput8 | SDL2 |
|--------------|------|
| Keyboard state | SDL_GetKeyboardState |
| Joystick axes | SDL_JoystickGetAxis |
| Joystick buttons | SDL_JoystickGetButton |
| Force feedback | SDL_HapticEffect |

## Build Instructions (Preview)

```bash
# Prerequisites
# 1. Install VS2022 with "Desktop development with C++" workload
# 2. Install vcpkg and integrate: vcpkg integrate install
# 3. Set VCPKG_ROOT environment variable

# Configure
cmake --preset=vs2022-debug

# Build
cmake --build --preset=vs2022-debug

# Or open in VS2022
start build/TopSpeed.sln
```

## File Changes Summary

### New Files
- `CMakeLists.txt` (root + each subproject)
- `CMakePresets.json`
- `vcpkg.json`
- `docs/BUILD.md`

### Modified Files (DxCommon)
- `Sound.cpp` - SDL2_mixer implementation
- `Input.cpp` - SDL2 input implementation
- `Game.cpp` - SDL2 window/init
- `Network.cpp` - Stub implementation
- `Application.cpp` - SDL2 init/quit
- Graphics files - Stub implementations

### Modified Files (TopSpeed)
- Multiplayer files - Conditional compilation guards

## Risks and Mitigations

| Risk | Mitigation |
|------|------------|
| SDL2_mixer lacks pitch control for engine sounds | May need SDL2 audio callback with pitch shifting, or accept limitation initially |
| Force feedback differences | SDL2 haptic API covers basics; complex .ffe effects may need simplification |
| MFC dependency in TopSpeed | Keep MFC for now (dialog-based app), or stub the dialog |

## Success Criteria

1. `cmake --preset=vs2022-debug` configures without errors
2. Project builds in VS2022 without errors
3. Executable launches and produces audio output
4. Keyboard/joystick input is recognized
5. Build process documented and reproducible

## Git Strategy

- Work in feature branch: `feature/cmake-sdl2-migration`
- Commit at logical checkpoints:
  1. Add CMake structure (compiles original code if possible)
  2. Add vcpkg integration
  3. Implement SDL2 audio wrapper
  4. Implement SDL2 input wrapper
  5. Stub graphics and network
  6. Final integration and testing
