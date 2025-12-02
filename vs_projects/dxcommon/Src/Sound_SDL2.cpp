/**
 * DXCommon library - SDL2 Sound Implementation
 * Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
 * SDL2 port 2025
 *
 * This program is distributed under the terms of the GNU General Public License version 3.
 */
#ifdef TOPSPEED_USE_SDL2

#include <DxCommon/If/Sound.h>
#include <Common/If/Tracer.h>
#include <cstring>
#include <cmath>

extern Tracer dxTracer;

namespace DirectX
{

//-----------------------------------------------------------------------------
// SoundManager Implementation
//-----------------------------------------------------------------------------

static bool g_sdlAudioInitialized = false;

SoundManager::SoundManager(::Window::Handle hwnd, UInt nChannels, UInt frequency, UInt bitrate)
    : m_directSound(nullptr)
    , m_created(false)
    , m_playInSoftware(false)
    , m_reverseStereo(false)
    , m_3dAlgorithm(AlgoDefault)
{
    if (!g_sdlAudioInitialized)
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
        {
            dxTracer.trace("SDL_InitSubSystem(AUDIO) failed: %s", SDL_GetError());
            return;
        }

        int mixerFlags = MIX_INIT_OGG;
        if ((Mix_Init(mixerFlags) & mixerFlags) != mixerFlags)
        {
            dxTracer.trace("Mix_Init failed: %s", Mix_GetError());
            return;
        }

        // Open audio device
        if (Mix_OpenAudio(frequency, AUDIO_S16SYS, nChannels, 1024) < 0)
        {
            dxTracer.trace("Mix_OpenAudio failed: %s", Mix_GetError());
            return;
        }

        // Allocate mixing channels (game may need many for sound effects)
        Mix_AllocateChannels(64);

        g_sdlAudioInitialized = true;
    }
    m_created = true;
    dxTracer.trace("SoundManager: SDL2_mixer initialized (freq=%d, channels=%d)", frequency, nChannels);
}

SoundManager::~SoundManager()
{
    if (g_sdlAudioInitialized)
    {
        Mix_CloseAudio();
        Mix_Quit();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        g_sdlAudioInitialized = false;
    }
}

Sound* SoundManager::create(Int resource, Boolean enable3d, UInt nBuffers)
{
    // Resource-based loading not implemented for SDL2
    dxTracer.trace("SoundManager::create(resource) - not implemented");
    return nullptr;
}

Sound* SoundManager::create(Char* filename, Boolean enable3d, UInt nBuffers)
{
    if (!filename)
        return nullptr;

    Mix_Chunk* chunk = Mix_LoadWAV(filename);
    if (!chunk)
    {
        dxTracer.trace("Failed to load sound: %s - %s", filename, Mix_GetError());
        return nullptr;
    }

    Sound* sound = new Sound(chunk, enable3d);
    sound->reverseStereo(m_reverseStereo);
    dxTracer.trace("Loaded sound: %s", filename);
    return sound;
}

Sound* SoundManager::create(DSBUFFERDESC& bufferDesc, Boolean enable3d, UInt nBuffers)
{
    // Buffer-based creation not implemented for SDL2
    dxTracer.trace("SoundManager::create(bufferDesc) - not implemented");
    return nullptr;
}

#ifdef _USE_VORBIS_
Sound* SoundManager::createVorbis(Char* filename, Boolean enable3d, UInt nBuffers)
{
    // SDL2_mixer handles OGG files natively via Mix_LoadWAV
    if (!filename)
        return nullptr;

    Mix_Chunk* chunk = Mix_LoadWAV(filename);
    if (!chunk)
    {
        dxTracer.trace("Failed to load vorbis sound: %s - %s", filename, Mix_GetError());
        return nullptr;
    }

    Sound* sound = new Sound(chunk, enable3d);
    sound->reverseStereo(m_reverseStereo);
    dxTracer.trace("Loaded vorbis sound: %s", filename);
    return sound;
}
#endif

Int SoundManager::bufferFormat(UInt nChannels, UInt frequency, UInt bitrate)
{
    // SDL2_mixer format is set at init time
    return dxSuccess;
}

Int SoundManager::listener3DInterface(LPDIRECTSOUND3DLISTENER* listener)
{
    // 3D audio stub
    if (listener)
        *listener = nullptr;
    return dxSuccess;
}

//-----------------------------------------------------------------------------
// Sound Implementation - Internal SDL2 Data
//-----------------------------------------------------------------------------

struct SDL2SoundData
{
    Mix_Chunk* chunk;
    int channel;
    int volume;      // 0-128 for SDL_mixer
    int pan;         // -100 to +100
    bool is3d;
    float posX, posY, posZ;
};

// Helper to get SDL2 data from Sound
static SDL2SoundData* GetSDL2Data(Sound* sound)
{
    if (!sound || !sound->buffer())
        return nullptr;
    return reinterpret_cast<SDL2SoundData*>(sound->buffer()[0]);
}

//-----------------------------------------------------------------------------
// Sound Implementation - Constructors
//-----------------------------------------------------------------------------

Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers, WaveFile* waveFile)
    : m_buffer(nullptr)
    , m_bufferSize(bufferSize)
    , m_nBuffers(nBuffers)
    , m_waveFile(waveFile)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    // DirectX compatibility constructor - not used with SDL2
}

Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers, LPWAVEFORMATEX waveFormat)
    : m_buffer(nullptr)
    , m_bufferSize(bufferSize)
    , m_nBuffers(nBuffers)
    , m_waveFile(nullptr)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    // DirectX compatibility constructor - not used with SDL2
}

#ifdef _USE_VORBIS_
Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers,
             OggVorbis_File* vorbisFile, UShort bitsPerSample, UInt avgBytesPerSec)
    : m_buffer(nullptr)
    , m_bufferSize(bufferSize)
    , m_nBuffers(nBuffers)
    , m_waveFile(nullptr)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    // Vorbis handled natively by SDL2_mixer, this constructor not used
}
#endif

// SDL2-specific constructor
Sound::Sound(Mix_Chunk* chunk, bool is3d)
    : m_buffer(new LPDIRECTSOUNDBUFFER[1])
    , m_bufferSize(0)
    , m_nBuffers(1)
    , m_waveFile(nullptr)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    SDL2SoundData* data = new SDL2SoundData();
    data->chunk = chunk;
    data->channel = -1;
    data->volume = MIX_MAX_VOLUME;
    data->pan = 0;
    data->is3d = is3d;
    data->posX = data->posY = data->posZ = 0.0f;

    m_buffer[0] = reinterpret_cast<LPDIRECTSOUNDBUFFER>(data);

    // Calculate length in seconds
    if (chunk && chunk->alen > 0)
    {
        // Approximate: assuming 44100Hz, 16-bit stereo
        m_length = static_cast<float>(chunk->alen) / (44100.0f * 2 * 2);
    }
}

Sound::~Sound()
{
    if (m_buffer)
    {
        SDL2SoundData* data = GetSDL2Data(this);
        if (data)
        {
            if (data->channel >= 0)
            {
                Mix_HaltChannel(data->channel);
            }
            if (data->chunk)
            {
                Mix_FreeChunk(data->chunk);
            }
            delete data;
        }
        delete[] m_buffer;
        m_buffer = nullptr;
    }
}

//-----------------------------------------------------------------------------
// Sound Implementation - Playback Methods
//-----------------------------------------------------------------------------

Int Sound::play(UInt priority, Boolean looped)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || !data->chunk)
        return dxFailed;

    int loops = looped ? -1 : 0;
    data->channel = Mix_PlayChannel(-1, data->chunk, loops);

    if (data->channel < 0)
    {
        dxTracer.trace("Mix_PlayChannel failed: %s", Mix_GetError());
        return dxFailed;
    }

    // Apply current volume and pan
    Mix_Volume(data->channel, data->volume);

    // Apply pan (-100 to +100 -> SDL2 panning)
    int left = 255, right = 255;
    if (data->pan < 0)
    {
        right = 255 + (data->pan * 255 / 100);
    }
    else if (data->pan > 0)
    {
        left = 255 - (data->pan * 255 / 100);
    }
    if (m_reverseStereo < 0)
    {
        int tmp = left;
        left = right;
        right = tmp;
    }
    Mix_SetPanning(data->channel, (Uint8)left, (Uint8)right);

    return dxSuccess;
}

Int Sound::stop()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return dxSuccess;

    Mix_HaltChannel(data->channel);
    data->channel = -1;
    return dxSuccess;
}

Int Sound::reset()
{
    stop();
    return dxSuccess;
}

Boolean Sound::playing()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return false;

    return Mix_Playing(data->channel) != 0;
}

//-----------------------------------------------------------------------------
// Sound Implementation - Control Methods
//-----------------------------------------------------------------------------

void Sound::pan(Int value)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return;

    // Clamp to -100..+100
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    data->pan = value;

    if (data->channel >= 0)
    {
        int left = 255, right = 255;
        if (value < 0)
        {
            right = 255 + (value * 255 / 100);
        }
        else if (value > 0)
        {
            left = 255 - (value * 255 / 100);
        }
        if (m_reverseStereo < 0)
        {
            int tmp = left;
            left = right;
            right = tmp;
        }
        Mix_SetPanning(data->channel, (Uint8)left, (Uint8)right);
    }
}

void Sound::frequency(Int value)
{
    // SDL2_mixer does not support frequency/pitch modification
    // This is a known limitation - noted in design document
    // Future: implement with SDL2 audio callback and pitch shifting
    // For now, log but don't spam
    static bool warned = false;
    if (!warned)
    {
        dxTracer.trace("Sound::frequency() - not implemented in SDL2 (pitch control unavailable)");
        warned = true;
    }
}

Int Sound::frequency()
{
    // Return default frequency
    return 44100;
}

void Sound::volume(Int value)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return;

    // Convert 0-100 to 0-128
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    data->volume = (value * MIX_MAX_VOLUME) / 100;

    if (data->channel >= 0)
    {
        Mix_Volume(data->channel, data->volume);
    }
}

Int Sound::volume()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return 0;

    return (data->volume * 100) / MIX_MAX_VOLUME;
}

//-----------------------------------------------------------------------------
// Sound Implementation - 3D Methods
//-----------------------------------------------------------------------------

Int Sound::initializeBuffer3D(UInt index)
{
    // 3D audio initialization stub
    return dxSuccess;
}

void Sound::position(Vector3 pos)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || !data->is3d)
        return;

    data->posX = pos.x;
    data->posY = pos.y;
    data->posZ = pos.z;

    // Simple 3D audio: pan based on X position, volume based on distance
    if (data->channel >= 0)
    {
        // Pan: map X position to -100..+100 (assuming reasonable range)
        int panValue = static_cast<int>(pos.x * 10.0f);
        if (panValue < -100) panValue = -100;
        if (panValue > 100) panValue = 100;

        int left = 255, right = 255;
        if (panValue < 0)
        {
            right = 255 + (panValue * 255 / 100);
        }
        else if (panValue > 0)
        {
            left = 255 - (panValue * 255 / 100);
        }
        Mix_SetPanning(data->channel, (Uint8)left, (Uint8)right);

        // Distance attenuation
        float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
        int vol = data->volume;
        if (dist > 1.0f)
        {
            vol = static_cast<int>(data->volume / dist);
            if (vol < 0) vol = 0;
        }
        Mix_Volume(data->channel, vol);
    }
}

//-----------------------------------------------------------------------------
// Sound Implementation - Low-level Methods (stubs)
//-----------------------------------------------------------------------------

Int Sound::fillBufferWithSound(LPDIRECTSOUNDBUFFER buffer)
{
    return dxSuccess;
}

Int Sound::fillBufferWithSilence(LPDIRECTSOUNDBUFFER buffer, LPWAVEFORMATEX waveFormat)
{
    return dxSuccess;
}

#ifdef _USE_VORBIS_
Int Sound::fillBufferWithSound(LPDIRECTSOUNDBUFFER buffer, OggVorbis_File* vorbisFile, UShort bitsPerSample)
{
    return dxSuccess;
}
#endif

LPDIRECTSOUNDBUFFER Sound::getFreeBuffer()
{
    return nullptr;
}

WAVEFORMATEX* Sound::waveFormat()
{
    return nullptr;
}

UInt Sound::copyBuffer(LPDIRECTSOUNDBUFFER* buffer, UInt bufferOffset, UInt bufferSize)
{
    return 0;
}

UInt Sound::insertSilence(UInt bufferOffset, UInt bufferSize)
{
    return 0;
}

Int Sound::restoreBuffer(LPDIRECTSOUNDBUFFER buffer, Boolean* wasRestored)
{
    if (wasRestored) *wasRestored = false;
    return dxSuccess;
}

//-----------------------------------------------------------------------------
// WaveFile Implementation (minimal stub for SDL2)
//-----------------------------------------------------------------------------

WaveFile::WaveFile()
    : m_waveFormat(nullptr)
    , m_mmioHandle(nullptr)
    , m_size(0)
    , m_flags(0)
    , m_fromMemory(false)
    , m_data(nullptr)
    , m_dataCurrent(nullptr)
    , m_dataSize(0)
    , m_buffer(nullptr)
{
    memset(&m_ckInfo, 0, sizeof(m_ckInfo));
    memset(&m_ckInfoRiff, 0, sizeof(m_ckInfoRiff));
    memset(&m_mmioInfo, 0, sizeof(m_mmioInfo));
}

WaveFile::~WaveFile()
{
    close();
}

Int WaveFile::open(Char* filename, WAVEFORMATEX* format, UInt flags)
{
    // SDL2_mixer handles file loading directly, WaveFile not needed
    return dxFailed;
}

Int WaveFile::openFromMemory(UByte* buffer, UInt bufferSize, WAVEFORMATEX* format, UInt flags)
{
    return dxFailed;
}

Int WaveFile::close()
{
    return dxSuccess;
}

Int WaveFile::read(UByte* pBuffer, UInt dwSizeToRead, UInt* pdwSizeRead)
{
    if (pdwSizeRead) *pdwSizeRead = 0;
    return dxFailed;
}

Int WaveFile::write(UInt nSizeToWrite, UByte* pbData, UInt* pnSizeWrote)
{
    if (pnSizeWrote) *pnSizeWrote = 0;
    return dxFailed;
}

Int WaveFile::resetFile()
{
    return dxSuccess;
}

Int WaveFile::readMmio()
{
    return dxFailed;
}

Int WaveFile::writeMmio(WAVEFORMATEX* pwfxDest)
{
    return dxFailed;
}

//-----------------------------------------------------------------------------
// Listener3D Implementation (stub)
//-----------------------------------------------------------------------------

Listener3D::Listener3D(SoundManager* soundManager)
    : m_ds3DListener(nullptr)
{
    memset(&m_parameters, 0, sizeof(m_parameters));
}

Listener3D::~Listener3D()
{
}

void Listener3D::parameters(Float dopplerFactor, Float rolloffFactor)
{
    // Stub - SDL2_mixer doesn't have built-in 3D listener
}

void Listener3D::position(Vector3 pos)
{
    // Stub
}

void Listener3D::orientation(Float dir)
{
    // Stub
}

void Listener3D::positionAndOrientation(Vector3 pos, Float dir)
{
    // Stub
}

} // namespace DirectX

#endif // TOPSPEED_USE_SDL2
