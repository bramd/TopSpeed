/**
 * SDL2 Compatibility Layer for DxCommon
 * Provides type definitions and macros to bridge DirectX and SDL2
 *
 * Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
 * SDL2 port 2025
 */
#ifndef __DXCOMMON_SDL2COMPAT_H__
#define __DXCOMMON_SDL2COMPAT_H__

#ifdef TOPSPEED_USE_SDL2

#include <SDL.h>
#include <SDL_mixer.h>
#include <windows.h>

// Stub DirectX types when using SDL2
typedef void* LPDIRECTSOUND8;
typedef void* LPDIRECTSOUNDBUFFER;
typedef void* LPDIRECTSOUND3DBUFFER;
typedef void* LPDIRECTSOUND3DLISTENER;
typedef void* LPDIRECTINPUT8;
typedef void* LPDIRECTINPUTDEVICE8;
typedef void* LPDIRECTINPUTEFFECT;
typedef void* LPDIRECT3DDEVICE8;

// Stub DirectX structures
struct DSBUFFERDESC { int dummy; };
struct DS3DBUFFER { int dummy; };
struct DS3DLISTENER { int dummy; };
struct WAVEFORMATEX {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned long nSamplesPerSec;
    unsigned long nAvgBytesPerSec;
    unsigned short nBlockAlign;
    unsigned short wBitsPerSample;
    unsigned short cbSize;
};
typedef WAVEFORMATEX* LPWAVEFORMATEX;

struct DIDEVICEINSTANCE { int dummy; };
struct DIDEVICEOBJECTINSTANCE { int dummy; };
struct DIFILEEFFECT { int dummy; };
typedef DIFILEEFFECT* LPCDIFILEEFFECT;

// DirectPlay stubs (multiplayer disabled)
#ifdef TOPSPEED_DISABLE_MULTIPLAYER
typedef void* IDirectPlay8Server;
typedef void* IDirectPlay8Client;
typedef void* PDIRECTPLAY8ADDRESS;
typedef void* IDirectPlay8Address;
struct DPN_APPLICATION_DESC { int dummy; };
typedef unsigned long DPNHANDLE;
#endif

// D3D type stubs
struct D3DDISPLAYMODE { int Width; int Height; int RefreshRate; int Format; };
struct D3DPRESENT_PARAMETERS { int dummy; };
typedef int D3DFORMAT;

// D3D color macro stub
#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) ((unsigned int)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

// MMIO types for WaveFile compatibility
#ifndef HMMIO
typedef void* HMMIO;
struct MMCKINFO { int dummy; };
struct MMIOINFO { int dummy; };
#endif

// D3DX types
typedef DWORD D3DXFONT_DESC;

#endif // TOPSPEED_USE_SDL2

#endif // __DXCOMMON_SDL2COMPAT_H__
