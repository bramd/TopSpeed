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
    Src/Sound_SDL2.cpp
    Src/Input_SDL2.cpp
    Src/Stubs_SDL2.cpp
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
