# DxCommon static library (SDL2 backend)
# Note: For SDL2 mode, we exclude DirectX-only source files that include
# headers like Dxerr8.h, dmusici.h, etc. These are replaced by SDL2-specific
# implementations and stubs.

add_library(DxCommon STATIC
    # Common utilities (no DirectX dependencies)
    Src/Common.cpp
    Src/Timer.cpp

    # SDL2 implementations
    Src/Sound_SDL2.cpp
    Src/Input_SDL2.cpp
    Src/Stubs_SDL2.cpp

    # The following files are excluded in SDL2 mode because they require DirectX:
    # Src/Application.cpp - requires Dxerr8.h, Direct3D8
    # Src/Game.cpp - requires Dxerr8.h, Direct3D8
    # Src/D3DFont.cpp - requires Direct3D8
    # Src/D3DSprite.cpp - requires Direct3D8
    # Src/Input.cpp - requires DirectInput8
    # Src/Light.cpp - requires Direct3D8
    # Src/Line.cpp - requires Direct3D8
    # Src/Mesh.cpp - requires Direct3D8, D3DX8
    # Src/Music.cpp - requires DirectMusic, DirectSound
    # Src/Network.cpp - requires DirectPlay
    # Src/Particle.cpp - requires Direct3D8
    # Src/Sound.cpp - requires DirectSound
    # Src/Utilities.cpp - requires D3DX8
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
    winmm
)
