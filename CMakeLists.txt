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

# Add subdirectories in dependency order
add_subdirectory(vs_projects/common)
add_subdirectory(vs_projects/dxcommon)
add_subdirectory(vs_projects/topspeed)
