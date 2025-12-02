/**
* Top Speed 3 - SDL2 Entry Point
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*
* SDL2 migration: Simplified entry point without MFC dependencies
*/

#ifdef TOPSPEED_USE_SDL2

#include "stdafx.h"
#include "Game.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <Common/If/File.h>
#include <Common/If/Common.h>

// Global game instance
static Game* g_game = nullptr;

// External tracers (defined in Common, DxCommon, and Game.cpp)
extern Tracer commonTracer;
extern Tracer dxTracer;
extern Tracer _raceTracer;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
    {
        MessageBoxA(NULL, SDL_GetError(), "SDL Init Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Check for single instance
    HANDLE mutex = CreateMutexA(NULL, FALSE, "TSpeed_3");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxA(NULL, "Top Speed 3 is already running.", "Top Speed 3", MB_OK | MB_ICONWARNING);
        SDL_Quit();
        return 1;
    }

    // Read settings
    File* settings = new File("TopSpeed.cfg", File::read);
    Int enableTracing = 0;
    if (settings->opened())
    {
        settings->readInt("EnableTracing", enableTracing, 0);
    }
    else
    {
        SAFE_DELETE(settings);
        settings = new File("TopSpeed.cfg", File::create | File::write);
        if (settings->opened())
        {
            fprintf(settings->getStream(), "[Settings]\n");
            settings->writeKeyInt("EnableTracing", enableTracing);
            settings->writeKeyString("Multiplayer", "127.0.0.1");
        }
    }
    SAFE_DELETE(settings);

    // Setup tracing (like original TopSpeed.cpp)
    File* traceFile = nullptr;
    if (enableTracing)
    {
        traceFile = new File("TopSpeed.trc", File::read | File::write | File::create);
        commonTracer.enable();
        commonTracer.bind(traceFile);
        _raceTracer.enable();
        _raceTracer.bind(traceFile);
        dxTracer.enable();
        dxTracer.bind(traceFile);
    }

    // Create game window
    SDL_Window* window = SDL_CreateWindow(
        "Top Speed 3 - SDL2",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        MessageBoxA(NULL, SDL_GetError(), "Window Creation Error", MB_OK | MB_ICONERROR);
        SDL_Quit();
        return 1;
    }

    // Get the native Windows handle from SDL window
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    // Create and initialize game
    g_game = new Game();
    g_game->initialize(hwnd);

    // Main game loop - Game::run() processes ONE frame, so we need to call it repeatedly
    // Note: InputManager::update() calls SDL_PollEvent() internally for joystick hotplug,
    // and Keyboard::update() uses SDL_PumpEvents() + SDL_GetKeyboardState() for key state.
    // We use SDL_PeekEvent here to check for quit without consuming events.
    bool running = true;
    SDL_Event event;

    while (running)
    {
        // Check for quit events without consuming them
        // (InputManager::update will process other events)
        while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_QUIT, SDL_QUIT) > 0)
        {
            running = false;
        }
        while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_WINDOWEVENT, SDL_WINDOWEVENT) > 0)
        {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                running = false;
        }

        // Run one frame of the game (this calls InputManager::update which pumps events)
        if (running)
        {
            g_game->run();
        }

        // Small delay to prevent 100% CPU usage
        SDL_Delay(1);
    }

    // Cleanup
    delete g_game;
    g_game = nullptr;

    SAFE_DELETE(traceFile);

    if (mutex)
    {
        CloseHandle(mutex);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

#endif // TOPSPEED_USE_SDL2
