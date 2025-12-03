/**
 * DXCommon library - SDL2 Input Implementation
 * Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
 * SDL2 port 2025
 *
 * This program is distributed under the terms of the GNU General Public License version 3.
 */
#ifdef TOPSPEED_USE_SDL2

#include <DxCommon/If/Input.h>
#include <DxCommon/If/DIKCodes.h>
#include <Common/If/Tracer.h>
#include <SDL.h>
#include <cstring>

extern Tracer dxTracer;

namespace DirectX
{

static bool g_sdlInputInitialized = false;
static SDL_Joystick* g_joystick = nullptr;
static SDL_Haptic* g_haptic = nullptr;

//-----------------------------------------------------------------------------
// Keyboard Implementation
//-----------------------------------------------------------------------------

Keyboard::Keyboard(LPDIRECTINPUTDEVICE8 device)
    : m_device(device)
    , m_available(0)
{
    std::memset(m_keys, 0, sizeof(m_keys));
    std::memset(&m_state, 0, sizeof(m_state));
}

Keyboard::~Keyboard()
{
}

Int Keyboard::update()
{
    SDL_PumpEvents();

    const Uint8* sdlKeys = SDL_GetKeyboardState(nullptr);

    // Clear all keys
    std::memset(m_state.keys, 0, sizeof(m_state.keys));

    // Map SDL scancodes to DirectInput key codes
    // Arrow keys
    m_state.keys[DIK_UP] = sdlKeys[SDL_SCANCODE_UP];
    m_state.keys[DIK_DOWN] = sdlKeys[SDL_SCANCODE_DOWN];
    m_state.keys[DIK_LEFT] = sdlKeys[SDL_SCANCODE_LEFT];
    m_state.keys[DIK_RIGHT] = sdlKeys[SDL_SCANCODE_RIGHT];

    // Common game keys
    m_state.keys[DIK_SPACE] = sdlKeys[SDL_SCANCODE_SPACE];
    m_state.keys[DIK_RETURN] = sdlKeys[SDL_SCANCODE_RETURN];
    m_state.keys[DIK_ESCAPE] = sdlKeys[SDL_SCANCODE_ESCAPE];
    m_state.keys[DIK_TAB] = sdlKeys[SDL_SCANCODE_TAB];
    m_state.keys[DIK_BACK] = sdlKeys[SDL_SCANCODE_BACKSPACE];

    // Letter keys A-Z
    m_state.keys[DIK_A] = sdlKeys[SDL_SCANCODE_A];
    m_state.keys[DIK_B] = sdlKeys[SDL_SCANCODE_B];
    m_state.keys[DIK_C] = sdlKeys[SDL_SCANCODE_C];
    m_state.keys[DIK_D] = sdlKeys[SDL_SCANCODE_D];
    m_state.keys[DIK_E] = sdlKeys[SDL_SCANCODE_E];
    m_state.keys[DIK_F] = sdlKeys[SDL_SCANCODE_F];
    m_state.keys[DIK_G] = sdlKeys[SDL_SCANCODE_G];
    m_state.keys[DIK_H] = sdlKeys[SDL_SCANCODE_H];
    m_state.keys[DIK_I] = sdlKeys[SDL_SCANCODE_I];
    m_state.keys[DIK_J] = sdlKeys[SDL_SCANCODE_J];
    m_state.keys[DIK_K] = sdlKeys[SDL_SCANCODE_K];
    m_state.keys[DIK_L] = sdlKeys[SDL_SCANCODE_L];
    m_state.keys[DIK_M] = sdlKeys[SDL_SCANCODE_M];
    m_state.keys[DIK_N] = sdlKeys[SDL_SCANCODE_N];
    m_state.keys[DIK_O] = sdlKeys[SDL_SCANCODE_O];
    m_state.keys[DIK_P] = sdlKeys[SDL_SCANCODE_P];
    m_state.keys[DIK_Q] = sdlKeys[SDL_SCANCODE_Q];
    m_state.keys[DIK_R] = sdlKeys[SDL_SCANCODE_R];
    m_state.keys[DIK_S] = sdlKeys[SDL_SCANCODE_S];
    m_state.keys[DIK_T] = sdlKeys[SDL_SCANCODE_T];
    m_state.keys[DIK_U] = sdlKeys[SDL_SCANCODE_U];
    m_state.keys[DIK_V] = sdlKeys[SDL_SCANCODE_V];
    m_state.keys[DIK_W] = sdlKeys[SDL_SCANCODE_W];
    m_state.keys[DIK_X] = sdlKeys[SDL_SCANCODE_X];
    m_state.keys[DIK_Y] = sdlKeys[SDL_SCANCODE_Y];
    m_state.keys[DIK_Z] = sdlKeys[SDL_SCANCODE_Z];

    // Number keys 0-9
    m_state.keys[DIK_0] = sdlKeys[SDL_SCANCODE_0];
    m_state.keys[DIK_1] = sdlKeys[SDL_SCANCODE_1];
    m_state.keys[DIK_2] = sdlKeys[SDL_SCANCODE_2];
    m_state.keys[DIK_3] = sdlKeys[SDL_SCANCODE_3];
    m_state.keys[DIK_4] = sdlKeys[SDL_SCANCODE_4];
    m_state.keys[DIK_5] = sdlKeys[SDL_SCANCODE_5];
    m_state.keys[DIK_6] = sdlKeys[SDL_SCANCODE_6];
    m_state.keys[DIK_7] = sdlKeys[SDL_SCANCODE_7];
    m_state.keys[DIK_8] = sdlKeys[SDL_SCANCODE_8];
    m_state.keys[DIK_9] = sdlKeys[SDL_SCANCODE_9];

    // Function keys F1-F12
    m_state.keys[DIK_F1] = sdlKeys[SDL_SCANCODE_F1];
    m_state.keys[DIK_F2] = sdlKeys[SDL_SCANCODE_F2];
    m_state.keys[DIK_F3] = sdlKeys[SDL_SCANCODE_F3];
    m_state.keys[DIK_F4] = sdlKeys[SDL_SCANCODE_F4];
    m_state.keys[DIK_F5] = sdlKeys[SDL_SCANCODE_F5];
    m_state.keys[DIK_F6] = sdlKeys[SDL_SCANCODE_F6];
    m_state.keys[DIK_F7] = sdlKeys[SDL_SCANCODE_F7];
    m_state.keys[DIK_F8] = sdlKeys[SDL_SCANCODE_F8];
    m_state.keys[DIK_F9] = sdlKeys[SDL_SCANCODE_F9];
    m_state.keys[DIK_F10] = sdlKeys[SDL_SCANCODE_F10];
    m_state.keys[DIK_F11] = sdlKeys[SDL_SCANCODE_F11];
    m_state.keys[DIK_F12] = sdlKeys[SDL_SCANCODE_F12];

    // Modifier keys
    m_state.keys[DIK_LSHIFT] = sdlKeys[SDL_SCANCODE_LSHIFT];
    m_state.keys[DIK_RSHIFT] = sdlKeys[SDL_SCANCODE_RSHIFT];
    m_state.keys[DIK_LCONTROL] = sdlKeys[SDL_SCANCODE_LCTRL];
    m_state.keys[DIK_RCONTROL] = sdlKeys[SDL_SCANCODE_RCTRL];
    m_state.keys[DIK_LALT] = sdlKeys[SDL_SCANCODE_LALT];
    m_state.keys[DIK_RALT] = sdlKeys[SDL_SCANCODE_RALT];

    // Numpad
    m_state.keys[DIK_NUMPAD0] = sdlKeys[SDL_SCANCODE_KP_0];
    m_state.keys[DIK_NUMPAD1] = sdlKeys[SDL_SCANCODE_KP_1];
    m_state.keys[DIK_NUMPAD2] = sdlKeys[SDL_SCANCODE_KP_2];
    m_state.keys[DIK_NUMPAD3] = sdlKeys[SDL_SCANCODE_KP_3];
    m_state.keys[DIK_NUMPAD4] = sdlKeys[SDL_SCANCODE_KP_4];
    m_state.keys[DIK_NUMPAD5] = sdlKeys[SDL_SCANCODE_KP_5];
    m_state.keys[DIK_NUMPAD6] = sdlKeys[SDL_SCANCODE_KP_6];
    m_state.keys[DIK_NUMPAD7] = sdlKeys[SDL_SCANCODE_KP_7];
    m_state.keys[DIK_NUMPAD8] = sdlKeys[SDL_SCANCODE_KP_8];
    m_state.keys[DIK_NUMPAD9] = sdlKeys[SDL_SCANCODE_KP_9];
    m_state.keys[DIK_NUMPADENTER] = sdlKeys[SDL_SCANCODE_KP_ENTER];

    // Navigation
    m_state.keys[DIK_HOME] = sdlKeys[SDL_SCANCODE_HOME];
    m_state.keys[DIK_END] = sdlKeys[SDL_SCANCODE_END];
    m_state.keys[DIK_PRIOR] = sdlKeys[SDL_SCANCODE_PAGEUP];  // Page Up
    m_state.keys[DIK_NEXT] = sdlKeys[SDL_SCANCODE_PAGEDOWN]; // Page Down
    m_state.keys[DIK_INSERT] = sdlKeys[SDL_SCANCODE_INSERT];
    m_state.keys[DIK_DELETE] = sdlKeys[SDL_SCANCODE_DELETE];

    // Copy to m_keys for compatibility
    std::memcpy(m_keys, m_state.keys, sizeof(m_keys));

    // Map keyboard keys to joystick-like controls for menu navigation
    // This matches the original DirectInput keyboard behavior
    m_state.x = 0;
    m_state.y = 0;
    m_state.z = 0;
    m_state.rx = 0;
    m_state.ry = 0;
    m_state.rz = 0;
    m_state.slider1 = 0;
    m_state.slider2 = 0;

    // Button mappings (matching original Input.cpp)
    m_state.b1 = sdlKeys[SDL_SCANCODE_RETURN] || sdlKeys[SDL_SCANCODE_KP_ENTER];
    m_state.b2 = sdlKeys[SDL_SCANCODE_RSHIFT] != 0;
    m_state.b3 = sdlKeys[SDL_SCANCODE_RALT] != 0;
    m_state.b4 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b5 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b6 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b7 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b8 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b9 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b10 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b11 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b12 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b13 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b14 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b15 = sdlKeys[SDL_SCANCODE_SPACE] != 0;
    m_state.b16 = sdlKeys[SDL_SCANCODE_SPACE] != 0;

    // POV (D-pad) mappings from arrow keys (matching original Input.cpp)
    m_state.pov1 = sdlKeys[SDL_SCANCODE_UP] != 0;
    m_state.pov2 = sdlKeys[SDL_SCANCODE_RIGHT] != 0;
    m_state.pov3 = sdlKeys[SDL_SCANCODE_DOWN] != 0;
    m_state.pov4 = sdlKeys[SDL_SCANCODE_LEFT] != 0;
    m_state.pov5 = sdlKeys[SDL_SCANCODE_UP] != 0;
    m_state.pov6 = sdlKeys[SDL_SCANCODE_RIGHT] != 0;
    m_state.pov7 = sdlKeys[SDL_SCANCODE_DOWN] != 0;
    m_state.pov8 = sdlKeys[SDL_SCANCODE_LEFT] != 0;

    return dxSuccess;
}

Int Keyboard::dataFormat()
{
    return dxSuccess;
}

Int Keyboard::cooperativeLevel(::Window::Handle handle, UInt flags)
{
    return dxSuccess;
}

//-----------------------------------------------------------------------------
// Joystick Implementation
//-----------------------------------------------------------------------------

Joystick::Joystick(LPDIRECTINPUTDEVICE8 device)
    : m_device(device)
    , m_available(0)
    , m_forceFeedbackCapable(false)
{
    std::memset(&m_state, 0, sizeof(m_state));
}

Joystick::~Joystick()
{
}

Int Joystick::update()
{
    if (!g_joystick)
        return dxFailed;

    SDL_JoystickUpdate();

    // Read axes (normalized to -1000..+1000 like DirectInput)
    int numAxes = SDL_JoystickNumAxes(g_joystick);

    if (numAxes > 0) m_state.x = SDL_JoystickGetAxis(g_joystick, 0) * 1000 / 32767;
    if (numAxes > 1) m_state.y = SDL_JoystickGetAxis(g_joystick, 1) * 1000 / 32767;
    if (numAxes > 2) m_state.z = SDL_JoystickGetAxis(g_joystick, 2) * 1000 / 32767;
    if (numAxes > 3) m_state.rx = SDL_JoystickGetAxis(g_joystick, 3) * 1000 / 32767;
    if (numAxes > 4) m_state.ry = SDL_JoystickGetAxis(g_joystick, 4) * 1000 / 32767;
    if (numAxes > 5) m_state.rz = SDL_JoystickGetAxis(g_joystick, 5) * 1000 / 32767;

    // Read buttons
    int numButtons = SDL_JoystickNumButtons(g_joystick);
    m_state.b1 = numButtons > 0 ? SDL_JoystickGetButton(g_joystick, 0) != 0 : false;
    m_state.b2 = numButtons > 1 ? SDL_JoystickGetButton(g_joystick, 1) != 0 : false;
    m_state.b3 = numButtons > 2 ? SDL_JoystickGetButton(g_joystick, 2) != 0 : false;
    m_state.b4 = numButtons > 3 ? SDL_JoystickGetButton(g_joystick, 3) != 0 : false;
    m_state.b5 = numButtons > 4 ? SDL_JoystickGetButton(g_joystick, 4) != 0 : false;
    m_state.b6 = numButtons > 5 ? SDL_JoystickGetButton(g_joystick, 5) != 0 : false;
    m_state.b7 = numButtons > 6 ? SDL_JoystickGetButton(g_joystick, 6) != 0 : false;
    m_state.b8 = numButtons > 7 ? SDL_JoystickGetButton(g_joystick, 7) != 0 : false;
    m_state.b9 = numButtons > 8 ? SDL_JoystickGetButton(g_joystick, 8) != 0 : false;
    m_state.b10 = numButtons > 9 ? SDL_JoystickGetButton(g_joystick, 9) != 0 : false;
    m_state.b11 = numButtons > 10 ? SDL_JoystickGetButton(g_joystick, 10) != 0 : false;
    m_state.b12 = numButtons > 11 ? SDL_JoystickGetButton(g_joystick, 11) != 0 : false;
    m_state.b13 = numButtons > 12 ? SDL_JoystickGetButton(g_joystick, 12) != 0 : false;
    m_state.b14 = numButtons > 13 ? SDL_JoystickGetButton(g_joystick, 13) != 0 : false;
    m_state.b15 = numButtons > 14 ? SDL_JoystickGetButton(g_joystick, 14) != 0 : false;
    m_state.b16 = numButtons > 15 ? SDL_JoystickGetButton(g_joystick, 15) != 0 : false;

    // Read POV/hat
    int numHats = SDL_JoystickNumHats(g_joystick);
    if (numHats > 0)
    {
        Uint8 hat = SDL_JoystickGetHat(g_joystick, 0);
        m_state.pov1 = (hat & SDL_HAT_UP) != 0;
        m_state.pov2 = (hat & SDL_HAT_RIGHT) != 0;
        m_state.pov3 = (hat & SDL_HAT_DOWN) != 0;
        m_state.pov4 = (hat & SDL_HAT_LEFT) != 0;
    }

    return dxSuccess;
}

Int Joystick::setRange(UInt axis, Int min, Int max)
{
    // SDL2 joystick axes are always -32768 to 32767
    // We scale in update() to match the expected range
    return dxSuccess;
}

Int Joystick::dataFormat()
{
    return dxSuccess;
}

Int Joystick::cooperativeLevel(::Window::Handle handle)
{
    return dxSuccess;
}

void Joystick::autocenter(Boolean b)
{
    // Force feedback autocenter
    if (g_haptic)
    {
        SDL_HapticSetAutocenter(g_haptic, b ? 50 : 0);
    }
}

//-----------------------------------------------------------------------------
// InputManager Implementation
//-----------------------------------------------------------------------------

InputManager::InputManager()
    : m_initialized(false)
    , m_directInput(nullptr)
    , m_joystick(nullptr)
    , m_keyboard(nullptr)
    , m_handle(nullptr)
{
}

InputManager::~InputManager()
{
    finalize();
}

Int InputManager::initialize(Window::Handle handle)
{
    if (m_initialized)
        return dxSuccess;

    m_handle = handle;

    if (!g_sdlInputInitialized)
    {
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) < 0)
        {
            dxTracer.trace("SDL_InitSubSystem(JOYSTICK|HAPTIC) failed: %s", SDL_GetError());
            // Continue without joystick
        }
        g_sdlInputInitialized = true;
    }

    // Create keyboard
    m_keyboard = new Keyboard(nullptr);
    dxTracer.trace("InputManager: Keyboard initialized");

    // Try to open first joystick
    int numJoysticks = SDL_NumJoysticks();
    dxTracer.trace("InputManager: Found %d joystick(s)", numJoysticks);

    if (numJoysticks > 0)
    {
        g_joystick = SDL_JoystickOpen(0);
        if (g_joystick)
        {
            m_joystick = new Joystick(nullptr);

            const char* name = SDL_JoystickName(g_joystick);
            dxTracer.trace("InputManager: Opened joystick: %s", name ? name : "Unknown");
            dxTracer.trace("  Axes: %d, Buttons: %d, Hats: %d",
                          SDL_JoystickNumAxes(g_joystick),
                          SDL_JoystickNumButtons(g_joystick),
                          SDL_JoystickNumHats(g_joystick));

            // Check for force feedback
            if (SDL_JoystickIsHaptic(g_joystick))
            {
                g_haptic = SDL_HapticOpenFromJoystick(g_joystick);
                if (g_haptic)
                {
                    m_joystick->FFcapable(true);
                    unsigned int supported = SDL_HapticQuery(g_haptic);
                    dxTracer.trace("InputManager: Haptic feedback available (0x%X)", supported);

                    // Initialize rumble if supported
                    if (SDL_HapticRumbleSupported(g_haptic))
                    {
                        SDL_HapticRumbleInit(g_haptic);
                    }
                }
            }
        }
    }

    m_initialized = true;
    return dxSuccess;
}

void InputManager::finalize()
{
    if (!m_initialized)
        return;

    delete m_keyboard;
    m_keyboard = nullptr;

    delete m_joystick;
    m_joystick = nullptr;

    if (g_haptic)
    {
        SDL_HapticClose(g_haptic);
        g_haptic = nullptr;
    }

    if (g_joystick)
    {
        SDL_JoystickClose(g_joystick);
        g_joystick = nullptr;
    }

    if (g_sdlInputInitialized)
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
        g_sdlInputInitialized = false;
    }

    m_initialized = false;
}

Int InputManager::update()
{
    // Process SDL events to keep input state current
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // Handle quit event
        if (event.type == SDL_QUIT)
        {
            // Could signal quit to application
        }
        // Joystick hotplug events
        else if (event.type == SDL_JOYDEVICEADDED)
        {
            if (!g_joystick)
            {
                g_joystick = SDL_JoystickOpen(event.jdevice.which);
                if (g_joystick && !m_joystick)
                {
                    m_joystick = new Joystick(nullptr);
                    dxTracer.trace("Joystick connected: %s", SDL_JoystickName(g_joystick));
                }
            }
        }
        else if (event.type == SDL_JOYDEVICEREMOVED)
        {
            if (g_joystick && SDL_JoystickInstanceID(g_joystick) == event.jdevice.which)
            {
                dxTracer.trace("Joystick disconnected");
                if (g_haptic)
                {
                    SDL_HapticClose(g_haptic);
                    g_haptic = nullptr;
                }
                SDL_JoystickClose(g_joystick);
                g_joystick = nullptr;
            }
        }
    }

    if (m_keyboard)
        m_keyboard->update();

    if (m_joystick)
        m_joystick->update();

    return dxSuccess;
}

const Input::State InputManager::state()
{
    Input::State combined;
    std::memset(&combined, 0, sizeof(combined));

    // Get keyboard state (includes button/POV mappings from arrow keys)
    if (m_keyboard)
    {
        const Input::State& kb = m_keyboard->state();
        std::memcpy(combined.keys, kb.keys, sizeof(combined.keys));
        // Copy keyboard button/POV mappings
        combined.b1 = kb.b1;
        combined.b2 = kb.b2;
        combined.b3 = kb.b3;
        combined.b4 = kb.b4;
        combined.b5 = kb.b5;
        combined.b6 = kb.b6;
        combined.b7 = kb.b7;
        combined.b8 = kb.b8;
        combined.b9 = kb.b9;
        combined.b10 = kb.b10;
        combined.b11 = kb.b11;
        combined.b12 = kb.b12;
        combined.b13 = kb.b13;
        combined.b14 = kb.b14;
        combined.b15 = kb.b15;
        combined.b16 = kb.b16;
        combined.pov1 = kb.pov1;
        combined.pov2 = kb.pov2;
        combined.pov3 = kb.pov3;
        combined.pov4 = kb.pov4;
        combined.pov5 = kb.pov5;
        combined.pov6 = kb.pov6;
        combined.pov7 = kb.pov7;
        combined.pov8 = kb.pov8;
    }

    // Merge joystick state (OR with keyboard so either input works)
    if (m_joystick)
    {
        const Input::State& js = m_joystick->state();
        combined.x = js.x;
        combined.y = js.y;
        combined.z = js.z;
        combined.rx = js.rx;
        combined.ry = js.ry;
        combined.rz = js.rz;
        combined.slider1 = js.slider1;
        combined.slider2 = js.slider2;
        combined.b1 = combined.b1 || js.b1;
        combined.b2 = combined.b2 || js.b2;
        combined.b3 = combined.b3 || js.b3;
        combined.b4 = combined.b4 || js.b4;
        combined.b5 = combined.b5 || js.b5;
        combined.b6 = combined.b6 || js.b6;
        combined.b7 = combined.b7 || js.b7;
        combined.b8 = combined.b8 || js.b8;
        combined.b9 = combined.b9 || js.b9;
        combined.b10 = combined.b10 || js.b10;
        combined.b11 = combined.b11 || js.b11;
        combined.b12 = combined.b12 || js.b12;
        combined.b13 = combined.b13 || js.b13;
        combined.b14 = combined.b14 || js.b14;
        combined.b15 = combined.b15 || js.b15;
        combined.b16 = combined.b16 || js.b16;
        combined.pov1 = combined.pov1 || js.pov1;
        combined.pov2 = combined.pov2 || js.pov2;
        combined.pov3 = combined.pov3 || js.pov3;
        combined.pov4 = combined.pov4 || js.pov4;
        combined.pov5 = combined.pov5 || js.pov5;
        combined.pov6 = combined.pov6 || js.pov6;
        combined.pov7 = combined.pov7 || js.pov7;
        combined.pov8 = combined.pov8 || js.pov8;
    }

    return combined;
}

// Stub callbacks (not used with SDL2)
BOOL CALLBACK InputManager::enumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    return FALSE;
}

BOOL CALLBACK InputManager::enumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
    return FALSE;
}

BOOL CALLBACK InputManager::enumKeyboardsCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    return FALSE;
}

//-----------------------------------------------------------------------------
// ForceFeedbackEffect Implementation
//-----------------------------------------------------------------------------

ForceFeedbackEffect::ForceFeedbackEffect(Joystick* joystick)
    : m_joystick(joystick)
{
}

ForceFeedbackEffect::ForceFeedbackEffect(Joystick* joystick, char* filename)
    : m_joystick(joystick)
{
    if (!filename || !g_haptic)
    {
        if (filename)
            dxTracer.trace("ForceFeedbackEffect: No haptic device for '%s'", filename);
        return;
    }

    // Extract effect name from filename
    const char* name = filename;
    const char* lastSlash = strrchr(filename, '/');
    const char* lastBackslash = strrchr(filename, '\\');
    if (lastSlash && (!lastBackslash || lastSlash > lastBackslash))
        name = lastSlash + 1;
    else if (lastBackslash)
        name = lastBackslash + 1;

    dxTracer.trace("ForceFeedbackEffect: Creating effect for '%s'", name);

    SDL_HapticEffect effect;
    SDL_memset(&effect, 0, sizeof(effect));
    unsigned int supported = SDL_HapticQuery(g_haptic);

    bool effectCreated = false;
    if (strstr(name, "crash") != nullptr && (supported & SDL_HAPTIC_SINE))
    {
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.period = 50;
        effect.periodic.magnitude = 32767;
        effect.periodic.length = 500;
        effect.periodic.fade_length = 100;
        effectCreated = true;
    }
    else if (strstr(name, "bumpleft") != nullptr && (supported & SDL_HAPTIC_SINE))
    {
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.direction.dir[0] = 27000;
        effect.periodic.period = 40;
        effect.periodic.magnitude = 24000;
        effect.periodic.length = 150;
        effect.periodic.fade_length = 50;
        effectCreated = true;
    }
    else if (strstr(name, "bumpright") != nullptr && (supported & SDL_HAPTIC_SINE))
    {
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.direction.dir[0] = 9000;
        effect.periodic.period = 40;
        effect.periodic.magnitude = 24000;
        effect.periodic.length = 150;
        effect.periodic.fade_length = 50;
        effectCreated = true;
    }
    else if (strstr(name, "curbleft") != nullptr && (supported & SDL_HAPTIC_SINE))
    {
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.direction.dir[0] = 27000;
        effect.periodic.period = 116;
        effect.periodic.magnitude = 16000;
        effect.periodic.length = SDL_HAPTIC_INFINITY;
        effectCreated = true;
    }
    else if (strstr(name, "curbright") != nullptr && (supported & SDL_HAPTIC_SINE))
    {
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.direction.dir[0] = 9000;
        effect.periodic.period = 116;
        effect.periodic.magnitude = 16000;
        effect.periodic.length = SDL_HAPTIC_INFINITY;
        effectCreated = true;
    }
    else if (strstr(name, "engine") != nullptr)
    {
        if (supported & SDL_HAPTIC_TRIANGLE)
            effect.type = SDL_HAPTIC_TRIANGLE;
        else if (supported & SDL_HAPTIC_SINE)
            effect.type = SDL_HAPTIC_SINE;
        else
            return;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.period = 100;
        effect.periodic.magnitude = 8000;
        effect.periodic.length = SDL_HAPTIC_INFINITY;
        effectCreated = true;
    }
    else if (strstr(name, "spring") != nullptr && (supported & SDL_HAPTIC_SPRING))
    {
        effect.type = SDL_HAPTIC_SPRING;
        effect.condition.direction.type = SDL_HAPTIC_POLAR;
        effect.condition.length = SDL_HAPTIC_INFINITY;
        effect.condition.right_sat[0] = 0xFFFF;
        effect.condition.left_sat[0] = 0xFFFF;
        effect.condition.right_coeff[0] = 0x4000;
        effect.condition.left_coeff[0] = 0x4000;
        effectCreated = true;
    }
    else if (strstr(name, "carstart") != nullptr && (supported & SDL_HAPTIC_SINE))
    {
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.period = 80;
        effect.periodic.magnitude = 20000;
        effect.periodic.length = 800;
        effect.periodic.attack_length = 200;
        effect.periodic.fade_length = 300;
        effectCreated = true;
    }
    else if (strstr(name, "gravel") != nullptr && (supported & SDL_HAPTIC_SINE))
    {
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.period = 30;
        effect.periodic.magnitude = 12000;
        effect.periodic.length = SDL_HAPTIC_INFINITY;
        effectCreated = true;
    }
    else if (supported & SDL_HAPTIC_SINE)
    {
        dxTracer.trace("ForceFeedbackEffect: Unknown '%s', using generic", name);
        effect.type = SDL_HAPTIC_SINE;
        effect.periodic.direction.type = SDL_HAPTIC_POLAR;
        effect.periodic.period = 50;
        effect.periodic.magnitude = 16000;
        effect.periodic.length = 300;
        effectCreated = true;
    }

    if (effectCreated)
    {
        int effectId = SDL_HapticNewEffect(g_haptic, &effect);
        if (effectId >= 0)
        {
            m_effects.push_back(reinterpret_cast<LPDIRECTINPUTEFFECT>(static_cast<intptr_t>(effectId)));
            dxTracer.trace("ForceFeedbackEffect: Created effect ID %d", effectId);
        }
        else
        {
            dxTracer.trace("ForceFeedbackEffect: Failed to create: %s", SDL_GetError());
        }
    }
}

ForceFeedbackEffect::~ForceFeedbackEffect()
{
    stop();
    if (g_haptic)
    {
        for (auto& effect : m_effects)
        {
            int effectId = static_cast<int>(reinterpret_cast<intptr_t>(effect));
            SDL_HapticDestroyEffect(g_haptic, effectId);
        }
    }
    m_effects.clear();
}

void ForceFeedbackEffect::play()
{
    if (!g_haptic || m_effects.empty())
        return;

    for (auto& effect : m_effects)
    {
        int effectId = static_cast<int>(reinterpret_cast<intptr_t>(effect));
        if (SDL_HapticRunEffect(g_haptic, effectId, 1) < 0)
        {
            dxTracer.trace("ForceFeedbackEffect::play: Failed %d: %s", effectId, SDL_GetError());
        }
    }
}

void ForceFeedbackEffect::stop()
{
    if (!g_haptic)
        return;

    for (auto& effect : m_effects)
    {
        int effectId = static_cast<int>(reinterpret_cast<intptr_t>(effect));
        SDL_HapticStopEffect(g_haptic, effectId);
    }
}

void ForceFeedbackEffect::gain(UInt gain)
{
    if (!g_haptic || m_effects.empty())
        return;

    // DirectInput gain is 0-10000, SDL2 uses 0-100
    int sdlGain = static_cast<int>(gain / 100);
    if (sdlGain > 100) sdlGain = 100;
    if (sdlGain < 0) sdlGain = 0;
    SDL_HapticSetGain(g_haptic, sdlGain);
}

void ForceFeedbackEffect::addFileEffect(LPCDIFILEEFFECT fileEffect)
{
    // Not implemented for SDL2
}

BOOL CALLBACK ForceFeedbackEffect::EnumAndCreateEffectsCallback(LPCDIFILEEFFECT effect, VOID* classRef)
{
    return FALSE;
}

} // namespace DirectX

#endif // TOPSPEED_USE_SDL2
