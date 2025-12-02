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
target_precompile_headers(TopSpeed PRIVATE StdAfx.h)
