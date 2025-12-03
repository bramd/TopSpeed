/**
* DXCommon library - SDL2 Stub Implementations
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*
* SDL2 migration: Stub implementations for graphics and network components
* that are not yet ported to SDL2. These provide minimal no-op implementations
* to allow the codebase to compile and run with audio/input functionality.
*/

#ifdef TOPSPEED_USE_SDL2

#include <DxCommon/If/D3DFont.h>
#include <DxCommon/If/Mesh.h>
#include <DxCommon/If/Light.h>
#include <DxCommon/If/Particle.h>
#include <DxCommon/If/Network.h>
#include <DxCommon/If/Game.h>

// Use DXCOMMON for tracing - define TRACE_DEBUG as a wrapper
#define TRACE_DEBUG DXCOMMON

namespace DirectX
{

// ============================================================================
// D3DFont Stubs - Text rendering not supported in SDL2 mode
// ============================================================================

D3DFont::D3DFont(Char* fontName, UInt size, UInt flags)
    : m_size(size)
    , m_flags(flags)
    , m_d3dDevice(nullptr)
    , m_texture(nullptr)
    , m_vertexBuffer(nullptr)
    , m_savedStateBlock(0)
    , m_drawTextStateBlock(0)
    , m_textureWidth(0)
    , m_textureHeight(0)
    , m_scale(1.0f)
{
    if (fontName)
        strncpy(m_fontName, fontName, sizeof(m_fontName) - 1);
    else
        m_fontName[0] = '\0';
    TRACE_DEBUG("D3DFont: Created stub font '%s' size %d", m_fontName, size);
}

D3DFont::~D3DFont()
{
}

Int D3DFont::initDevices(LPDIRECT3DDEVICE8 d3dDevice)
{
    m_d3dDevice = d3dDevice;
    return dxSuccess;
}

Int D3DFont::restoreDevices()
{
    return dxSuccess;
}

Int D3DFont::invalidateDevices()
{
    return dxSuccess;
}

Int D3DFont::deleteDevices()
{
    return dxSuccess;
}

Int D3DFont::drawText(Float x, Float y, UInt color, Char* text, Int flags)
{
    // Stub - no rendering in SDL2 mode (audio-only game doesn't need text display)
    return dxSuccess;
}

Int D3DFont::draw3DText(Char* text, Int flags)
{
    // Stub - no 3D text rendering
    return dxSuccess;
}

SIZE D3DFont::size(Char* text)
{
    SIZE s = { 0, 0 };
    if (text)
    {
        s.cx = (LONG)(strlen(text) * m_size * 0.6f);  // Rough estimate
        s.cy = (LONG)m_size;
    }
    return s;
}


// ============================================================================
// Mesh Stubs - 3D mesh rendering not supported in SDL2 mode
// ============================================================================

Mesh::Mesh(const Char* name)
    : m_sysMemMesh(nullptr)
    , m_localMesh(nullptr)
    , m_nMaterials(0)
    , m_materials(nullptr)
    , m_textures(nullptr)
    , m_useMaterials(false)
{
    if (name)
        strncpy(m_name, name, sizeof(m_name) - 1);
    else
        m_name[0] = '\0';
}

Mesh::~Mesh()
{
    destroy();
}

Int Mesh::render(LPDIRECT3DDEVICE8 d3dDevice, Boolean drawOpaqueSubsets, Boolean drawAlphaSubsets, Float scale)
{
    return dxSuccess;
}

Int Mesh::renderAt(LPDIRECT3DDEVICE8 d3dDevice, Float x, Float y, Float z, Float rx, Float ry, Float rz,
                   Boolean drawOpaqueSubsets, Boolean drawAlphaSubsets, Float scale)
{
    return dxSuccess;
}

Int Mesh::renderAt(LPDIRECT3DDEVICE8 d3dDevice, Vector3 pos, Boolean drawOpaqueSubsets, Boolean drawAlphaSubsets)
{
    return dxSuccess;
}

Int Mesh::renderAt(LPDIRECT3DDEVICE8 d3dDevice, Vector3 pos, Vector3 att, Boolean drawOpaqueSubsets, Boolean drawAlphaSubsets)
{
    return dxSuccess;
}

Int Mesh::renderAtCurrent(LPDIRECT3DDEVICE8 d3dDevice, Boolean drawOpaqueSubsets, Boolean drawAlphaSubsets)
{
    return dxSuccess;
}

Int Mesh::renderShadowAtCurrent(LPDIRECT3DDEVICE8 d3dDevice, Vector3& light, Vector3& u, Vector3& v, Vector3& w)
{
    return dxSuccess;
}

Int Mesh::renderShadowAt(LPDIRECT3DDEVICE8 d3dDevice, Vector3& pos, Vector3& att,
                         Vector3& light, Vector3& u, Vector3& v, Vector3& w)
{
    return dxSuccess;
}

Int Mesh::setFVF(LPDIRECT3DDEVICE8 d3dDevice, UInt FVF)
{
    return dxSuccess;
}

Int Mesh::restoreDevices(LPDIRECT3DDEVICE8 d3dDevice)
{
    return dxSuccess;
}

Int Mesh::invalidateDevices()
{
    return dxSuccess;
}

Int Mesh::create(LPDIRECT3DDEVICE8 d3dDevice, Char* filename)
{
    TRACE_DEBUG("Mesh: Stub create from file '%s'", filename ? filename : "(null)");
    return dxSuccess;
}

Int Mesh::create(LPDIRECT3DDEVICE8 d3dDevice, Char* filename, Char* textureFile)
{
    TRACE_DEBUG("Mesh: Stub create from file '%s' with texture '%s'",
                filename ? filename : "(null)", textureFile ? textureFile : "(null)");
    return dxSuccess;
}

Int Mesh::create(LPDIRECT3DDEVICE8 d3dDevice, LPDIRECTXFILEDATA fileData)
{
    return dxSuccess;
}

Int Mesh::destroy()
{
    m_sysMemMesh = nullptr;
    m_localMesh = nullptr;
    if (m_materials)
    {
        delete[] m_materials;
        m_materials = nullptr;
    }
    if (m_textures)
    {
        delete[] m_textures;
        m_textures = nullptr;
    }
    m_nMaterials = 0;
    return dxSuccess;
}

Int Mesh::setTexture(LPDIRECT3DDEVICE8 d3dDevice, Char* filename)
{
    return dxSuccess;
}


// ============================================================================
// Frame Stubs
// ============================================================================

Frame::Frame(const Char* name)
    : m_mesh(nullptr)
    , m_next(nullptr)
    , m_child(nullptr)
{
    if (name)
        strncpy(m_name, name, sizeof(m_name) - 1);
    else
        m_name[0] = '\0';
    m_meshName[0] = '\0';
    memset(&m_matrix, 0, sizeof(m_matrix));
}

Frame::~Frame()
{
    destroy();
}

Mesh* Frame::findMesh(Char* meshName)
{
    return nullptr;
}

Frame* Frame::findFrame(Char* frameName)
{
    return nullptr;
}

Boolean Frame::enumMeshes(Boolean (*enumMeshCallback)(Mesh*, void*), void* context)
{
    return false;
}

Int Frame::destroy()
{
    if (m_mesh)
    {
        delete m_mesh;
        m_mesh = nullptr;
    }
    if (m_child)
    {
        delete m_child;
        m_child = nullptr;
    }
    if (m_next)
    {
        delete m_next;
        m_next = nullptr;
    }
    return dxSuccess;
}

Int Frame::restoreDevices(LPDIRECT3DDEVICE8 d3dDevice)
{
    return dxSuccess;
}

Int Frame::invalidateDevices()
{
    return dxSuccess;
}

Int Frame::render(LPDIRECT3DDEVICE8 d3dDevice, Boolean drawOpaqueSubsets, Boolean drawAlphaSubsets, D3DXMATRIX* matWorldMatrix)
{
    return dxSuccess;
}


// ============================================================================
// MeshFile Stubs
// ============================================================================

Int MeshFile::create(LPDIRECT3DDEVICE8 d3dDevice, Char* filename)
{
    TRACE_DEBUG("MeshFile: Stub create from file '%s'", filename ? filename : "(null)");
    return dxSuccess;
}

Int MeshFile::createFromResource(LPDIRECT3DDEVICE8 d3dDevice, Char* resource, Char* type)
{
    return dxSuccess;
}

Int MeshFile::render(LPDIRECT3DDEVICE8 d3dDevice, D3DXMATRIX* matWorldMatrix)
{
    return dxSuccess;
}

Int MeshFile::loadMesh(LPDIRECT3DDEVICE8 d3dDevice, LPDIRECTXFILEDATA fileData, Frame* parentFrame)
{
    return dxSuccess;
}

Int MeshFile::loadFrame(LPDIRECT3DDEVICE8 d3dDevice, LPDIRECTXFILEDATA fileData, Frame* parentFrame)
{
    return dxSuccess;
}


// ============================================================================
// Light Stubs
// ============================================================================

Light::Light(Type type, Vector3 position, Vector3 direction)
{
    memset(&m_light, 0, sizeof(m_light));
}

Light::Light(Type type, Vector3 position, Vector3 direction, Int color)
{
    memset(&m_light, 0, sizeof(m_light));
}

Light::~Light()
{
}

void Light::enable(LPDIRECT3DDEVICE8 d3dDevice, UInt number)
{
    // Stub - no lighting in SDL2 mode
}

void Light::disable(LPDIRECT3DDEVICE8 d3dDevice, UInt number)
{
    // Stub
}


// ============================================================================
// ParticleSystem Stubs
// ============================================================================

ParticleSystem::ParticleSystem(LPDIRECT3DDEVICE8 d3dDevice, UInt nparticles,
                               D3DFORMAT textureFormat, Char* textureFile, Game* game)
    : m_base(0)
    , m_flush(0)
    , m_discard(0)
    , m_nParticles(0)
    , m_nMaxParticles(nparticles)
    , m_particles(nullptr)
    , m_particlesFree(nullptr)
    , m_game(game)
    , m_d3dDevice(d3dDevice)
    , m_vertexBuffer(nullptr)
    , m_texture(nullptr)
{
}

ParticleSystem::~ParticleSystem()
{
    if (m_particles)
    {
        delete[] m_particles;
        m_particles = nullptr;
    }
}

void ParticleSystem::emitFadingSingle(Vector3 position, UInt color, Float sceneTime)
{
    // Stub - no particle rendering
}

void ParticleSystem::prepareNextFrame(Float sceneTime, Float elapsed)
{
    // Stub
}

Int ParticleSystem::restoreDevices(LPDIRECT3DDEVICE8 d3dDevice)
{
    return dxSuccess;
}

Int ParticleSystem::invalidateDevices()
{
    return dxSuccess;
}

Int ParticleSystem::render()
{
    return dxSuccess;
}


// ============================================================================
// Game::Control3D Stubs
// ============================================================================

Game::Control3D::Control3D(Game* game, LPDIRECT3DDEVICE8 device, Int screenWidth, Int screenHeight)
    : m_game(game)
    , m_d3dDevice(device)
    , m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_angle(0.0f)
{
    memset(&m_viewMatrix, 0, sizeof(m_viewMatrix));
}

Game::Control3D::~Control3D()
{
}

void Game::Control3D::view(Float fromx, Float fromy, Float fromz, Float tox, Float toy, Float toz)
{
    // Stub - no 3D view in SDL2 mode
}

void Game::Control3D::view(Vector3 from, Vector3 to)
{
    // Stub
}

void Game::Control3D::projection(Float Near, Float Far)
{
    // Stub
}

void Game::Control3D::projection(Float Near, Float Far, Float angle)
{
    m_angle = angle;
}

void Game::Control3D::material(UInt color, Float alpha)
{
    // Stub
}

void Game::Control3D::ambient(UInt color)
{
    // Stub
}

void Game::Control3D::alpha(Boolean enable)
{
    // Stub
}

void Game::Control3D::antialiasing(Boolean enable)
{
    // Stub
}

void Game::Control3D::position(LPDIRECT3DDEVICE8 d3dDevice, Float x, Float y, Float z)
{
    // Stub
}

void Game::Control3D::position(LPDIRECT3DDEVICE8 d3dDevice, Vector3 pos)
{
    // Stub
}

void Game::Control3D::yawPitchRoll(LPDIRECT3DDEVICE8 d3dDevice, Float yaw, Float pitch, Float roll)
{
    // Stub
}

void Game::Control3D::yawPitchRoll(LPDIRECT3DDEVICE8 d3dDevice, Vector3 ypr)
{
    // Stub
}

void Game::Control3D::rotateAxis(LPDIRECT3DDEVICE8 d3dDevice, DirectX::Vector3 axis, Float angle)
{
    // Stub
}

void Game::Control3D::scale(LPDIRECT3DDEVICE8 d3dDevice, Float scale)
{
    // Stub
}

void Game::Control3D::scale(LPDIRECT3DDEVICE8 d3dDevice, Float scaleX, Float scaleY, Float scaleZ)
{
    // Stub
}

void Game::Control3D::reset(LPDIRECT3DDEVICE8 d3dDevice)
{
    // Stub
}


#ifdef TOPSPEED_DISABLE_MULTIPLAYER
// ============================================================================
// Network Stubs - Only when multiplayer is disabled
// ============================================================================

Server::Server()
    : m_directPlayServer(nullptr)
    , m_directPlayAddress(nullptr)
    , m_iServer(nullptr)
    , m_started(false)
{
    memset(&m_directPlayAppDesc, 0, sizeof(m_directPlayAppDesc));
    memset(&m_applicationGUID, 0, sizeof(m_applicationGUID));
}

Server::~Server()
{
    stopSession();
}

Int Server::startSession(Char* name, UInt port)
{
    TRACE_DEBUG("Server: Multiplayer disabled - cannot start session '%s'", name ? name : "(null)");
    return dxFailed;
}

void Server::stopSession()
{
    m_started = false;
}

void Server::sendPacket(UInt to, void* buffer, UInt size, Boolean secure, UInt timeout)
{
    // Stub - no networking
}

HRESULT WINAPI Server::directPlayMessageHandler(void* pvUserContext, DWORD dwMessageId, void* pMsgBuffer)
{
    return S_OK;
}

HRESULT WINAPI Server::localMessageHandler(UInt messageId, void* msgBuffer)
{
    return S_OK;
}


Client::Client()
    : m_directPlayClient(nullptr)
    , m_iClient(nullptr)
    , m_directPlayEnumHandle(0)
    , m_nSessions(0)
{
    memset(&m_applicationGUID, 0, sizeof(m_applicationGUID));
    memset(m_sessions, 0, sizeof(m_sessions));
}

Client::~Client()
{
    finalize();
}

Int Client::initialize()
{
    TRACE_DEBUG("Client: Multiplayer disabled - network initialization skipped");
    return dxSuccess;
}

Int Client::finalize()
{
    m_nSessions = 0;
    return dxSuccess;
}

Int Client::sendPacket(void* buffer, UInt size, Boolean secure, UInt timeout)
{
    return dxFailed;
}

Int Client::startSessionEnum(UInt port, const Char* ipaddress)
{
    TRACE_DEBUG("Client: Multiplayer disabled - cannot enumerate sessions");
    return dxFailed;
}

Int Client::stopSessionEnum()
{
    return dxSuccess;
}

Int Client::session(UInt i, SessionInfo& info)
{
    return dxFailed;
}

Int Client::joinSession(UInt i)
{
    return dxFailed;
}

Int Client::joinSessionAt(UInt port, const Char* ipaddress)
{
    return dxFailed;
}

void Client::sessionEnumCallback(const DPN_APPLICATION_DESC* appDesc,
                                  IDirectPlay8Address* addressHost,
                                  IDirectPlay8Address* addressDevice)
{
    // Stub
}

HRESULT WINAPI Client::directPlayMessageHandler(void* pvUserContext, DWORD dwMessageId, void* pMsgBuffer)
{
    return S_OK;
}

HRESULT WINAPI Client::localMessageHandler(UInt messageId, void* msgBuffer)
{
    return S_OK;
}

#endif // TOPSPEED_DISABLE_MULTIPLAYER


} // namespace DirectX

#endif // TOPSPEED_USE_SDL2
