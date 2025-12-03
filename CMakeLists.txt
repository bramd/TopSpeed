cmake_minimum_required(VERSION 3.21)
project(TopSpeed VERSION 3.0.0 LANGUAGES C CXX)

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

# Platform-specific setup
if(EMSCRIPTEN)
    message(STATUS "Building for Emscripten/WebAssembly")

    # Use Emscripten's SDL2 port
    set(USE_FLAGS "-s USE_SDL=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")

    # ASYNCIFY for synchronous file loading support
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASYNCIFY=1")
    # Include IDBFS for persistent storage to IndexedDB
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lidbfs.js")

    # Allow memory growth for audio buffers
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH=1")

    # Export as HTML
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    message(STATUS "Building for native platform")
    # Find SDL2 via vcpkg for native builds
    find_package(SDL2 CONFIG REQUIRED)
endif()

# Add subdirectories in dependency order
add_subdirectory(vs_projects/Common)
add_subdirectory(vs_projects/DxCommon)
add_subdirectory(vs_projects/topspeed)
