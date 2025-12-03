/**
* Top Speed 3 - SDL2 Entry Point
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*
* SDL2 migration: Simplified entry point without MFC dependencies
*/

#ifdef TOPSPEED_USE_SDL2

#ifndef __EMSCRIPTEN__
#include "stdafx.h"
#endif
#include "Game.h"
#include <SDL.h>
#include <Common/If/File.h>
#include <Common/If/Common.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <SDL_syswm.h>
#endif

// Global game instance
static Game* g_game = nullptr;
static SDL_Window* g_window = nullptr;
static bool g_running = true;

// External tracers (defined in Common, DxCommon, and Game.cpp)
extern Tracer commonTracer;
extern Tracer dxTracer;
extern Tracer _raceTracer;

#ifdef __EMSCRIPTEN__
// Sync the Emscripten filesystem to IndexedDB for persistence
// Call this after writing settings, highscores, etc.
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void syncFilesystemToIndexedDB()
    {
        EM_ASM(
            FS.syncfs(false, function(err) {
                if (err) {
                    console.error('Failed to sync filesystem to IndexedDB:', err);
                } else {
                    console.log('Filesystem synced to IndexedDB');
                }
            });
        );
    }
}

// Initialize IDBFS and load persisted data synchronously using ASYNCIFY
static void initializeIDBFS()
{
    printf("Initializing IDBFS for persistent storage...\n");
    EM_ASM(
        // Mount IDBFS on the current working directory
        FS.mount(FS.filesystems.IDBFS || IDBFS, {}, '/');

        // Sync FROM IndexedDB to memory (populate = true)
        // Using Asyncify to make this synchronous
        Asyncify.handleSleep(function(wakeUp) {
            FS.syncfs(true, function(err) {
                if (err) {
                    console.error('Failed to load from IndexedDB:', err);
                } else {
                    console.log('Loaded persisted data from IndexedDB');
                }
                wakeUp();
            });
        });
    );
    printf("IDBFS initialized\n");
}
#endif

// Main loop function for emscripten_set_main_loop
void main_loop_iteration()
{
    SDL_Event event;

    // Check for quit events
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_QUIT, SDL_QUIT) > 0)
    {
        g_running = false;
    }
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_WINDOWEVENT, SDL_WINDOWEVENT) > 0)
    {
        if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            g_running = false;
    }

    // Run one frame of the game
    if (g_running && g_game)
    {
        g_game->run();
    }

#ifdef __EMSCRIPTEN__
    if (!g_running)
    {
        emscripten_cancel_main_loop();
    }
#endif
}

#ifdef __EMSCRIPTEN__
// Emscripten entry point
int main(int argc, char* argv[])
#else
// Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
    {
#ifndef __EMSCRIPTEN__
        MessageBoxA(NULL, SDL_GetError(), "SDL Init Error", MB_OK | MB_ICONERROR);
#else
        printf("SDL Init Error: %s\n", SDL_GetError());
#endif
        return 1;
    }

#ifdef __EMSCRIPTEN__
    // Initialize IDBFS for persistent storage (settings, highscores)
    // This must happen before reading any config files
    initializeIDBFS();
#else
    // Check for single instance (Windows only)
    HANDLE mutex = CreateMutexA(NULL, FALSE, "TSpeed_3");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxA(NULL, "Top Speed 3 is already running.", "Top Speed 3", MB_OK | MB_ICONWARNING);
        SDL_Quit();
        return 1;
    }
#endif

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
#ifdef __EMSCRIPTEN__
            // Sync new config file to IndexedDB
            syncFilesystemToIndexedDB();
#endif
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
    g_window = SDL_CreateWindow(
        "Top Speed 3",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (!g_window)
    {
#ifndef __EMSCRIPTEN__
        MessageBoxA(NULL, SDL_GetError(), "Window Creation Error", MB_OK | MB_ICONERROR);
#else
        printf("Window Creation Error: %s\n", SDL_GetError());
#endif
        SDL_Quit();
        return 1;
    }

#ifdef __EMSCRIPTEN__
    // For Emscripten, we don't need a native window handle
    // The window handle is just used for sound/input initialization which SDL handles
    void* hwnd = nullptr;
#else
    // Get the native Windows handle from SDL window
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(g_window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;
#endif

    // Create and initialize game
    g_game = new Game();
    g_game->initialize(hwnd);

#ifdef __EMSCRIPTEN__
    // Set up main loop for browser (0 = use requestAnimationFrame, 1 = simulate infinite loop)
    emscripten_set_main_loop(main_loop_iteration, 0, 1);
#else
    // Native main loop
    while (g_running)
    {
        main_loop_iteration();
        SDL_Delay(1);
    }

    // Cleanup (only reached on native - Emscripten loop never returns)
    delete g_game;
    g_game = nullptr;

    SAFE_DELETE(traceFile);

    if (mutex)
    {
        CloseHandle(mutex);
    }
#endif

    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}

#endif // TOPSPEED_USE_SDL2
