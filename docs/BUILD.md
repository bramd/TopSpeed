# TopSpeed 3 - SDL2 Build Guide

This document describes how to build TopSpeed 3 using the modern CMake/SDL2 build system.

## Prerequisites

### Required Software

1. **Visual Studio 2022** (Community, Professional, or Enterprise)
   - Workloads: "Desktop development with C++"
   - Components: "C++ MFC for latest v143 build tools"

2. **CMake 3.21+** (usually bundled with VS2022)

3. **vcpkg** (package manager for C++ libraries)
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   ```

4. **Git** (for source control)

### Environment Variables

Set `VCPKG_ROOT` to your vcpkg installation:
```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

## Build Instructions

### Option 1: Command Line (Recommended)

```powershell
# Navigate to project root
cd F:\src\TopSpeed

# Configure with CMake preset
cmake --preset=default

# Build Debug configuration
cmake --build build --config Debug

# Build Release configuration
cmake --build build --config Release
```

### Option 2: Visual Studio 2022 IDE

1. Open Visual Studio 2022
2. File → Open → CMake...
3. Select `F:\src\TopSpeed\CMakeLists.txt`
4. VS will automatically configure using the presets
5. Select "Debug" or "Release" from the configuration dropdown
6. Build → Build All (Ctrl+Shift+B)

### Option 3: VS Code with CMake Tools

1. Open the TopSpeed folder in VS Code
2. Install the "CMake Tools" extension
3. Press F1 → "CMake: Select Configure Preset" → "default"
4. Press F1 → "CMake: Build"

## Project Structure

```
TopSpeed/
├── CMakeLists.txt           # Root CMake configuration
├── CMakePresets.json        # VS2022 presets with vcpkg
├── vcpkg.json               # Dependency manifest
├── vs_projects/
│   ├── common/
│   │   ├── CMakeLists.txt   # Common library
│   │   └── Src/             # Platform utilities
│   ├── dxcommon/
│   │   ├── CMakeLists.txt   # DxCommon library
│   │   ├── If/              # Headers
│   │   └── Src/             # SDL2 implementations
│   └── topspeed/
│       ├── CMakeLists.txt   # Main executable
│       └── *.cpp/*.h        # Game code
└── docs/
    ├── BUILD.md             # This file
    └── plans/               # Design documents
```

## Dependencies

Managed automatically by vcpkg:
- **SDL2** - Window management, input, audio, timing

Note: SDL2_mixer is no longer required. Audio is handled by a custom mixer using
SDL2's low-level audio API with SDL_AudioStream for real-time pitch control.

## Build Configurations

| Define | Description |
|--------|-------------|
| `TOPSPEED_USE_SDL2` | Use SDL2 instead of DirectX 8 |
| `TOPSPEED_DISABLE_MULTIPLAYER` | Disable DirectPlay networking |
| `COMMON_STATIC` | Build Common as static library |
| `DXCOMMON_STATIC` | Build DxCommon as static library |
| `_USE_VORBIS_` | Enable OGG/Vorbis audio support |

## Output Files

After building, executables are located in:
```
build/vs_projects/topspeed/Debug/TopSpeed.exe
build/vs_projects/topspeed/Release/TopSpeed.exe
```

## Troubleshooting

### vcpkg not found
Ensure `VCPKG_ROOT` environment variable is set and restart your terminal/IDE.

### MFC not found
Install the "C++ MFC for latest v143 build tools" component via VS Installer.

### SDL2 linking errors
vcpkg should handle this automatically. Try:
```powershell
vcpkg install sdl2:x64-windows
```

### CMake preset not found
Ensure you're using CMake 3.21+ and your IDE supports CMake presets (VS2022 17.0+).

## Known Limitations (SDL2 Mode)

1. **Graphics**: 3D rendering is stubbed out. The game runs in audio-only mode.
2. **Multiplayer**: DirectPlay networking is disabled.
3. **OGG/Vorbis**: Currently only WAV files are supported (OGG support can be added via libvorbis).

## Legacy DirectX 8 Build

The original VS2008 project files are preserved in `vs_projects/*/` with `.vcproj` extensions.
These require the DirectX SDK (August 2007) and Visual Studio 2008.

## CI Integration

For automated builds, use the CMake presets:

```yaml
# Example GitHub Actions
- name: Configure
  run: cmake --preset=default

- name: Build
  run: cmake --build build --config Release

- name: Test
  run: ctest --test-dir build --config Release
```
