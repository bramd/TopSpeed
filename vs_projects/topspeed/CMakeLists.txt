# TopSpeed executable (SDL2 mode)
# Note: For SDL2 mode, we exclude MFC-dependent files and use Main_SDL2.cpp instead

add_executable(TopSpeed WIN32
    # Core game logic (platform-independent)
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
    Track.cpp

    # SDL2 entry point (replaces MFC TopSpeed.cpp/TopSpeedDlg.cpp)
    Main_SDL2.cpp
    StdAfx.cpp

    # Resource file (for icon etc)
    TopSpeed.rc

    # Files excluded in SDL2 mode (require MFC):
    # TopSpeed.cpp - CTopSpeedApp MFC application class
    # TopSpeedDlg.cpp - CTopSpeedDlg MFC dialog class
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
)

target_link_libraries(TopSpeed PRIVATE
    DxCommon
    Common
)

# Precompiled header
target_precompile_headers(TopSpeed PRIVATE StdAfx.h)

# Set working directory for debugging to the game assets folder
# This allows the game to find Sounds/, Tracks/, etc. without copying
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
