/**
* Top Speed 3 - Multiplayer Stubs Header
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*
* Stub types for multiplayer functionality when building without network support
* (e.g., Emscripten/WASM builds)
*/

#ifndef __MULTIPLAYER_STUBS_H__
#define __MULTIPLAYER_STUBS_H__

#ifdef __EMSCRIPTEN__

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// Stub DirectX network types
namespace DirectX {

class Client {
public:
    struct SessionInfo {
        char sessionName[MAX_PATH];
    };
};

class IClient {
public:
    virtual ~IClient() {}
    virtual void onPacket(unsigned int from, void* buffer, unsigned int size) = 0;
    virtual void onSessionLost() = 0;
};

class IServer {
public:
    virtual ~IServer() {}
    virtual void onPacket(unsigned int from, void* buffer, unsigned int size) = 0;
    virtual void onAddConnection(unsigned int id) = 0;
    virtual void onRemoveConnection(unsigned int id) = 0;
    virtual void onSessionLost() = 0;
};

} // namespace DirectX

#endif // __EMSCRIPTEN__

#endif // __MULTIPLAYER_STUBS_H__
