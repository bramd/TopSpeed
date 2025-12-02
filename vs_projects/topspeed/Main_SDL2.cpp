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

    // Setup tracing
    File* traceFile = nullptr;
    if (enableTracing)
    {
        traceFile = new File("TopSpeed.trc", File::create | File::write);
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
    g_game->initialize(hwnd);  // Initialize with Windows HWND

    // Run the game (this contains its own main loop)
    g_game->run();

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
