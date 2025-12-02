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

#ifdef __EMSCRIPTEN__
// Emscripten/WASM - provide stub types for Windows-specific types
#include <cstdint>

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int32_t LONG;
typedef int BOOL;
typedef unsigned int UINT;
typedef void* HWND;
typedef long HRESULT;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef CHAR TCHAR;
typedef void* HANDLE;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef void VOID;
typedef unsigned char BYTE;
typedef float FLOAT;
typedef long LRESULT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef int64_t LONGLONG;
typedef char* LPSTR;
typedef const char* LPCSTR;

// LARGE_INTEGER for timer functions
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;

// CALLBACK/WINAPI macros (used for Windows callback functions)
#define CALLBACK
#define WINAPI

// MAX_PATH for path buffers
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// SIZE structure
typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE;

// GUID structure for DirectX compatibility
typedef struct _GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    unsigned char Data4[8];
} GUID;

// WAVEFORMATEX for audio format compatibility
typedef struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX;
typedef WAVEFORMATEX* LPWAVEFORMATEX;

// MMIO types for WAV file handling (stub - not actually used in WASM)
typedef void* HMMIO;
typedef struct {
    DWORD ckid;
    DWORD cksize;
    DWORD fccType;
    DWORD dwDataOffset;
    DWORD dwFlags;
} MMCKINFO;
typedef struct {
    DWORD dwFlags;
} MMIOINFO;

// Common Windows constants
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

// COM-style return codes
#ifndef S_OK
#define S_OK ((HRESULT)0)
#endif
#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005)
#endif

// Windows error codes
#define ERROR_NO_MORE_FILES 18L

// File finding stubs for WASM (will use Emscripten filesystem)
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

typedef struct _WIN32_FIND_DATA {
    DWORD dwFileAttributes;
    DWORD ftCreationTime[2];
    DWORD ftLastAccessTime[2];
    DWORD ftLastWriteTime[2];
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR  cFileName[MAX_PATH];
    CHAR  cAlternateFileName[14];
} WIN32_FIND_DATA;
typedef WIN32_FIND_DATA* LPWIN32_FIND_DATA;

// Stub implementations - will need to be replaced with Emscripten filesystem calls
inline HANDLE FindFirstFile(const char* pattern, WIN32_FIND_DATA* data) {
    (void)pattern; (void)data;
    return INVALID_HANDLE_VALUE; // Not implemented yet
}
inline BOOL FindNextFile(HANDLE handle, WIN32_FIND_DATA* data) {
    (void)handle; (void)data;
    return FALSE;
}
inline BOOL FindClose(HANDLE handle) {
    (void)handle;
    return TRUE;
}
inline DWORD GetLastError() { return ERROR_NO_MORE_FILES; }

#else
// Windows - include Windows headers for real types
#include <windows.h>
#include <mmsystem.h>  // For WAVEFORMATEX, HMMIO, MMCKINFO, etc.
#endif

// =============================================================================
// DirectSound stub types - these aren't in Windows SDK so we need to define them
// =============================================================================
typedef void* LPDIRECTSOUND8;
typedef void* LPDIRECTSOUNDBUFFER;
typedef void* LPDIRECTSOUND3DBUFFER;
typedef void* LPDIRECTSOUND3DLISTENER;

// DirectSound buffer structures (not in standard Windows SDK)
typedef struct _DSBUFFERDESC_STUB {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
} DSBUFFERDESC_STUB;
typedef DSBUFFERDESC_STUB DSBUFFERDESC;
typedef DSBUFFERDESC* LPDSBUFFERDESC;

typedef struct _DS3DBUFFER_STUB {
    DWORD dwSize;
} DS3DBUFFER_STUB;
typedef DS3DBUFFER_STUB DS3DBUFFER;
typedef DS3DBUFFER* LPDS3DBUFFER;

typedef struct _DS3DLISTENER_STUB {
    DWORD dwSize;
} DS3DLISTENER_STUB;
typedef DS3DLISTENER_STUB DS3DLISTENER;
typedef DS3DLISTENER* LPDS3DLISTENER;

// DirectSound constants
#define DS_OK 0
#define DSBCAPS_CTRL3D          0x00000010
#define DSBCAPS_CTRLFREQUENCY   0x00000020
#define DSBCAPS_CTRLPAN         0x00000040
#define DSBCAPS_CTRLVOLUME      0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY 0x00000100
#define DSBCAPS_STATIC          0x00000002
#define DSBCAPS_LOCSOFTWARE     0x00000008
#define DSBPLAY_LOOPING         0x00000001

// =============================================================================
// DirectInput stub types
// =============================================================================
typedef void* LPDIRECTINPUT8;
typedef void* LPDIRECTINPUTDEVICE8;
typedef void* LPDIRECTINPUTEFFECT;

typedef struct _DIDEVICEINSTANCE_STUB {
    DWORD dwSize;
    GUID guidInstance;
    GUID guidProduct;
    DWORD dwDevType;
    TCHAR tszInstanceName[260];
    TCHAR tszProductName[260];
} DIDEVICEINSTANCE_STUB;
typedef DIDEVICEINSTANCE_STUB DIDEVICEINSTANCE;
typedef DIDEVICEINSTANCE* LPDIDEVICEINSTANCE;

typedef struct _DIDEVICEOBJECTINSTANCE_STUB {
    DWORD dwSize;
} DIDEVICEOBJECTINSTANCE_STUB;
typedef DIDEVICEOBJECTINSTANCE_STUB DIDEVICEOBJECTINSTANCE;

typedef struct _DIFILEEFFECT_STUB {
    DWORD dwSize;
} DIFILEEFFECT_STUB;
typedef DIFILEEFFECT_STUB DIFILEEFFECT;
typedef DIFILEEFFECT* LPDIFILEEFFECT;
typedef const DIFILEEFFECT* LPCDIFILEEFFECT;

// =============================================================================
// Direct3D 8 stub types
// =============================================================================
typedef void* LPDIRECT3D8;
typedef void* LPDIRECT3DDEVICE8;
typedef void* LPDIRECT3DTEXTURE8;
typedef void* LPDIRECT3DVERTEXBUFFER8;
typedef void* LPDIRECTXFILEDATA;

// D3D8 structures
typedef struct _D3DDISPLAYMODE_STUB {
    UINT Width;
    UINT Height;
    UINT RefreshRate;
    int Format;
} D3DDISPLAYMODE_STUB;
typedef D3DDISPLAYMODE_STUB D3DDISPLAYMODE;

typedef struct _D3DPRESENT_PARAMETERS_STUB {
    UINT BackBufferWidth;
    UINT BackBufferHeight;
    int BackBufferFormat;
    UINT BackBufferCount;
    int MultiSampleType;
    int SwapEffect;
    HWND hDeviceWindow;
    BOOL Windowed;
    BOOL EnableAutoDepthStencil;
    int AutoDepthStencilFormat;
    DWORD Flags;
    UINT FullScreen_RefreshRateInHz;
    UINT FullScreen_PresentationInterval;
} D3DPRESENT_PARAMETERS_STUB;
typedef D3DPRESENT_PARAMETERS_STUB D3DPRESENT_PARAMETERS;

typedef struct _D3DLIGHT8_STUB {
    int Type;
    float Diffuse[4];
    float Specular[4];
    float Ambient[4];
    float Position[3];
    float Direction[3];
    float Range;
    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    float Theta;
    float Phi;
} D3DLIGHT8_STUB;
typedef D3DLIGHT8_STUB D3DLIGHT8;

// D3D light type constants
#define D3DLIGHT_POINT          1
#define D3DLIGHT_SPOT           2
#define D3DLIGHT_DIRECTIONAL    3

typedef struct _D3DMATERIAL8_STUB {
    struct { float r, g, b, a; } Diffuse;
    struct { float r, g, b, a; } Ambient;
    struct { float r, g, b, a; } Specular;
    struct { float r, g, b, a; } Emissive;
    float Power;
} D3DMATERIAL8_STUB;
typedef D3DMATERIAL8_STUB D3DMATERIAL8;

typedef int D3DFORMAT;

// D3D format constants
#define D3DFMT_UNKNOWN      0
#define D3DFMT_R5G6B5       23
#define D3DFMT_X1R5G5B5     24
#define D3DFMT_A1R5G5B5     25
#define D3DFMT_A4R4G4B4     26
#define D3DFMT_X8R8G8B8     22
#define D3DFMT_A8R8G8B8     21
#define D3DFMT_D16          80

// D3D constants
#define D3D_SDK_VERSION     220
#define D3DADAPTER_DEFAULT  0
#define D3DDEVTYPE_HAL      1
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING 0x00000020
#define D3DCLEAR_TARGET     0x00000001
#define D3DCLEAR_ZBUFFER    0x00000002
#define D3DSWAPEFFECT_DISCARD 1
#define D3DRTYPE_TEXTURE    3
#define D3DTS_VIEW          2
#define D3DTS_PROJECTION    3
#define D3DRS_AMBIENT       139

// D3D error codes
#define D3DERR_INVALIDCALL      ((HRESULT)0x8876086C)
#define D3DERR_OUTOFVIDEOMEMORY ((HRESULT)0x8876017C)

// D3DX types
typedef struct _D3DXMATRIX {
    float m[4][4];
} D3DXMATRIX;

typedef struct _D3DXVECTOR3 {
    float x, y, z;
    _D3DXVECTOR3() : x(0), y(0), z(0) {}
    _D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
} D3DXVECTOR3;

typedef DWORD D3DXFONT_DESC;

// D3DX FVF flags
#define D3DFVF_XYZ      0x002
#define D3DFVF_NORMAL   0x010
#define D3DFVF_TEX1     0x100

// D3D color macro
#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((DWORD)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

// D3DX stub functions (inline no-ops)
inline void D3DXMatrixLookAtLH(D3DXMATRIX*, const D3DXVECTOR3*, const D3DXVECTOR3*, const D3DXVECTOR3*) {}
inline void D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX*, float, float, float, float, float, float) {}
inline LPDIRECT3D8 Direct3DCreate8(UINT) { return nullptr; }

// DXGetErrorString8 stub
inline const char* DXGetErrorString8(HRESULT) { return "SDL2 mode - no DirectX errors"; }

// =============================================================================
// DirectPlay stubs (multiplayer disabled)
// =============================================================================
#ifdef TOPSPEED_DISABLE_MULTIPLAYER
typedef void* IDirectPlay8Server;
typedef void* IDirectPlay8Client;
typedef void* PDIRECTPLAY8ADDRESS;
typedef void* IDirectPlay8Address;

typedef struct _DPN_APPLICATION_DESC_STUB {
    DWORD dwSize;
    DWORD dwFlags;
    GUID guidInstance;
    GUID guidApplication;
    DWORD dwMaxPlayers;
    DWORD dwCurrentPlayers;
    WCHAR* pwszSessionName;
    WCHAR* pwszPassword;
    void* pvReservedData;
    DWORD dwReservedDataSize;
    void* pvApplicationReservedData;
    DWORD dwApplicationReservedDataSize;
} DPN_APPLICATION_DESC_STUB;
typedef DPN_APPLICATION_DESC_STUB DPN_APPLICATION_DESC;

typedef DWORD DPNHANDLE;
typedef DPNHANDLE* PDPNHANDLE;
#endif

// =============================================================================
// DirectMusic stub types (for Music.h compatibility)
// =============================================================================
typedef void* IDirectMusicLoader8;
typedef void* IDirectMusicPerformance8;
typedef void* IDirectMusicAudioPath8;
typedef void* IDirectMusicSegment8;
typedef void* IDirectMusicChordMap8;
typedef void* IDirectMusicStyle8;
typedef void* IDirectMusicScript8;

// DirectMusic audiopath constants
#define DMUS_APATH_DYNAMIC_3D           1
#define DMUS_APATH_DYNAMIC_MONO         2
#define DMUS_APATH_DYNAMIC_STEREO       6
#define DMUS_APATH_SHARED_STEREOPLUSREVERB 8

#endif // TOPSPEED_USE_SDL2

#endif // __DXCOMMON_SDL2COMPAT_H__
