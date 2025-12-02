# TopSpeed executable (SDL2 mode)
# Note: For SDL2 mode, we exclude MFC-dependent files and use Main_SDL2.cpp instead

# Core game logic (platform-independent)
set(TOPSPEED_SOURCES
    Car.cpp
    ComputerPlayer.cpp
    Game.cpp
    Level.cpp
    LevelSingleRace.cpp
    LevelTimeTrial.cpp
    Menu.cpp
    RaceInput.cpp
    RaceSettings.cpp
    Track.cpp

    # SDL2 entry point (replaces MFC TopSpeed.cpp/TopSpeedDlg.cpp)
    Main_SDL2.cpp
)

# Windows-only files (multiplayer uses DirectPlay, StdAfx is MSVC precompiled header)
if(NOT EMSCRIPTEN)
    list(APPEND TOPSPEED_SOURCES
        LevelMultiplayer.cpp
        NetworkPlayer.cpp
        RaceClient.cpp
        RaceServer.cpp
        StdAfx.cpp
    )
else()
    # Stubs for multiplayer classes (required because Game.cpp and Menu.cpp reference them)
    list(APPEND TOPSPEED_SOURCES
        Multiplayer_Stubs.cpp
    )
endif()

if(EMSCRIPTEN)
    # Emscripten: no WIN32 subsystem, no resource file
    add_executable(TopSpeed ${TOPSPEED_SOURCES})
else()
    # Windows: WIN32 subsystem + resource file
    add_executable(TopSpeed WIN32
        ${TOPSPEED_SOURCES}
        TopSpeed.rc
    )
endif()

target_include_directories(TopSpeed PRIVATE
    ${CMAKE_SOURCE_DIR}/vs_projects
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Platform-specific definitions
if(EMSCRIPTEN)
    target_compile_definitions(TopSpeed PRIVATE
        DXCOMMON_STATIC
        COMMON_STATIC
        _USE_VORBIS_
        TOPSPEED_USE_SDL2
        TOPSPEED_DISABLE_MULTIPLAYER
    )

    # Emscripten-specific linker flags
    set(GAME_ASSETS_DIR "${CMAKE_SOURCE_DIR}/installer/tspeed")
    set(RESOURCE_SOUNDS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Sounds")

    # Use custom HTML shell
    set_target_properties(TopSpeed PROPERTIES
        LINK_FLAGS "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )

    # Preload essential assets for startup
    # These are needed before the game can show the menu
    target_link_options(TopSpeed PRIVATE
        # Export callMain so we can start the game after user interaction
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['callMain']"
        # Preload resource sounds (embedded in .rc on Windows, files on WASM)
        # These include pitd_logo.wav, vehicle sounds, ambient sounds
        "SHELL:--preload-file ${RESOURCE_SOUNDS_DIR}@/Sounds"
        # Preload English menu sounds
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Sounds/en/menu@/Sounds/en/menu"
        # Preload number sounds (used everywhere)
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Sounds/en/numbers@/Sounds/en/numbers"
        # Preload music (background themes)
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Sounds/en/music@/Sounds/en/music"
        # Preload track names (needed for menu)
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Sounds/en/tracks@/Sounds/en/tracks"
        # Preload vehicle names (needed for menu)
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Sounds/en/vehicles@/Sounds/en/vehicles"
        # Preload race sounds (copilot callouts, countdown, etc.)
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Sounds/en/race@/Sounds/en/race"
        # Preload English language identifier
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Sounds/en.ogg@/Sounds/en.ogg"
        # Preload game data files
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Tracks@/Tracks"
        "SHELL:--preload-file ${GAME_ASSETS_DIR}/Vehicles@/Vehicles"
    )
else()
    target_compile_definitions(TopSpeed PRIVATE
        WIN32
        _WINDOWS
        DXCOMMON_STATIC
        COMMON_STATIC
        _USE_VORBIS_
        TOPSPEED_USE_SDL2
        TOPSPEED_DISABLE_MULTIPLAYER
    )
endif()

target_link_libraries(TopSpeed PRIVATE
    DxCommon
    Common
)

# Precompiled header (Windows only - Emscripten doesn't support MSVC PCH)
if(NOT EMSCRIPTEN)
    target_precompile_headers(TopSpeed PRIVATE StdAfx.h)
endif()

# Windows-specific settings
if(NOT EMSCRIPTEN)
    # Set working directory for debugging to the game assets folder
    set_target_properties(TopSpeed PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/installer/tspeed"
    )

    # Copy DLLs to assets folder for running/debugging
    add_custom_command(TARGET TopSpeed POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TopSpeed>
            "${CMAKE_SOURCE_DIR}/installer/tspeed/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:TopSpeed>
            "${CMAKE_SOURCE_DIR}/installer/tspeed/"
        COMMAND_EXPAND_LISTS
    )
endif()
