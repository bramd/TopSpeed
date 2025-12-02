# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Top Speed 3 is an audio-based racing game developed by Playing in the Dark (2003-2013). It's designed for accessibility, using sound as the primary feedback mechanism. The game supports single player modes (time trial, single race) and multiplayer via DirectPlay networking.

## Build Requirements

- Microsoft Visual Studio 2008 or later
- DirectX SDK (August 2007 or one with DirectPlay support) - place include/lib folders in `vs_projects/dxsdk`
- LibOGG (libogg-1.2.0) and LibVorbis (libvorbis-1.3.1) - place include folders (`ogg`, `vorbis`) in `vs_projects/`

## Build Order

The projects must be built in dependency order:

1. **CommonStatic** (`vs_projects/common/CommonStatic.sln`) → outputs `common.lib`
2. **DXCommonStatic** (`vs_projects/dxcommon/DxCommonStatic.sln`) → outputs `dxcommon.lib`
   - Requires `common.lib` in `vs_projects/dxcommon/` root
3. **libogg, libvorbis, libvorbisfile** (static builds)
4. **TopSpeed** (`vs_projects/topspeed/TopSpeed.sln`)
   - Requires in `vs_projects/topspeed/` root:
     - `CommonStatic.lib` (renamed from common.lib)
     - `DXCommonStatic.lib` (renamed from dxcommon.lib)
     - `ogg_static.lib`, `vorbis_static.lib`, `vorbisfile_static.lib`

Choose either Debug or Release configuration consistently across all projects.

## Architecture

### Library Layers

```
TopSpeed (game executable)
    ↓
DXCommon (DirectX wrapper library)
    ↓
Common (platform utilities)
```

- **Common** (`vs_projects/common/`): Platform-independent utilities - file I/O, threading (Mutex), networking, data structures (TList, TQueue), tracing/logging
- **DXCommon** (`vs_projects/dxcommon/`): DirectX 8 abstraction layer - sound management, input handling (keyboard, joystick, force feedback), 3D graphics, networking via DirectPlay
- **TopSpeed** (`vs_projects/topspeed/`): Game logic and content

### Game Structure

The main `Game` class (`vs_projects/topspeed/Game.h`) manages game state and owns:
- **Level subclasses**: `LevelTimeTrial`, `LevelSingleRace`, `LevelMultiplayer` - each game mode inherits from base `Level` class
- **Car**: Vehicle physics, engine sounds, force feedback effects, manual/automatic transmission
- **Track**: Course definition with road segments (curves, surfaces), ambient sounds, weather effects
- **Menu**: Audio-based menu navigation
- **RaceServer/RaceClient**: DirectPlay-based multiplayer

### Key Patterns

- Event system uses `EventList` (linked list of timed events) for scheduling sounds and game actions
- Sound-centric design: Most feedback is audio (co-pilot callouts, engine sounds, surface changes)
- Force feedback effects loaded from `.ffe` files in `installer/tspeed/Effects/`
- Tracks defined as sequences of `Track::Definition` (road type, surface, ambient noise)
- Localization via sound files organized by language code (e.g., `Sounds/En/`)

## Installer

Uses Inno Setup (`installer/TSPEED.ISS`). Game assets located in `installer/tspeed/`.
