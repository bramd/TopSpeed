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

// Resource IDs from TopSpeed resource.h - needed for lookup table
#define IDR_PITD_LOGO                   131
#define IDR_VEHICLE1B                   133
#define IDR_VEHICLE1C                   134
#define IDR_VEHICLE10E                  137
#define IDR_VEHICLE11E                  138
#define IDR_VEHICLE10S                  139
#define IDR_VEHICLE11S                  140
#define IDR_VEHICLE1S                   141
#define IDR_VEHICLE2S                   142
#define IDR_CRASH_SHORT                 143
#define IDR_VEHICLE1E                   144
#define IDR_VEHICLE2E                   145
#define IDR_CROWD                       146
#define IDR_OCEAN                       147
#define IDR_ASPHALT                     149
#define IDR_RAIN                        156
#define IDR_WIPERS                      157
#define IDR_DESERT                      158
#define IDR_AIRPLANE                    159
#define IDR_CLOCK                       160
#define IDR_VEHICLE1CM                  161
#define IDR_VEHICLE3E                   162
#define IDR_VEHICLE4E                   163
#define IDR_VEHICLE5E                   164
#define IDR_VEHICLE6E                   165
#define IDR_VEHICLE7E                   166
#define IDR_BUMP1                       168
#define IDR_VEHICLE10H                  169
#define IDR_VEHICLE1H                   170
#define IDR_VEHICLE2H                   171
#define IDR_VEHICLE3H                   172
#define IDR_VEHICLE4H                   173
#define IDR_VEHICLE5H                   174
#define IDR_VEHICLE6H                   175
#define IDR_BADSWITCH                   176
#define IDR_GRAVEL                      177
#define IDR_WATER                       178
#define IDR_SAND                        179
#define IDR_VEHICLE1T                   180
#define IDR_VEHICLE2T                   181
#define IDR_VEHICLE9E                   183
#define IDR_VEHICLE9S                   184
#define IDR_VEHICLE9T                   185
#define IDR_VEHICLE9H                   186
#define IDR_VEHICLE9F                   187
#define IDR_JET                         188
#define IDR_VEHICLE12E                  189
#define IDR_VEHICLE12S                  190
#define IDR_VEHICLE12H                  191
#define IDR_VEHICLE12T                  192
#define IDR_VEHICLE12F                  193
#define IDR_THUNDER                     194
#define IDR_VEHICLE3B                   195
#define IDR_PILE                        196
#define IDR_VEHICLE8E                   197
#define IDR_VEHICLE3C                   198
#define IDR_VEHICLE3CM                  199
#define IDR_VEHICLE9C                   200
#define IDR_VEHICLE9CM                  201
#define IDR_VEHICLE10C                  202
#define IDR_VEHICLE10CM                 203
#define IDR_ERROR                       204
#define IDR_SNOW                        205
#define IDR_CONST                       206
#define IDR_VEHICLE9B                   207
#define IDR_RIVER                       208
#define IDR_AIRPORT                     209
#define IDR_WIND                        210
#define IDR_HELICOPTER                  211
#define IDR_VEHICLE6B                   212
#define IDR_STORM                       213
#define IDR_UNKEY1                      214
#define IDR_UNKEY2                      215
#define IDR_UNKEY3                      216
#define IDR_UNKEY4                      217
#define IDR_UNKEY5                      218
#define IDR_UNKEY6                      219
#define IDR_UNKEY7                      220
#define IDR_UNKEY8                      221
#define IDR_UNKEY9                      222
#define IDR_UNKEY10                     223
#define IDR_UNKEY11                     224
#define IDR_UNKEY12                     225
#define IDR_OWL                         226

extern Tracer dxTracer;

// Lookup table: map resource IDs to external filenames
static const char* getResourceFilename(int resourceId)
{
    switch (resourceId)
    {
    case IDR_PITD_LOGO:    return "Sounds/pitd_logo.wav";
    case IDR_VEHICLE1B:    return "Sounds/vehicle1_b.wav";
    case IDR_VEHICLE1C:    return "Sounds/vehicle1_c.wav";
    case IDR_VEHICLE10E:   return "Sounds/vehicle10_e.wav";
    case IDR_VEHICLE11E:   return "Sounds/vehicle11_e.wav";
    case IDR_VEHICLE10S:   return "Sounds/vehicle10_s.wav";
    case IDR_VEHICLE11S:   return "Sounds/vehicle11_s.wav";
    case IDR_VEHICLE1S:    return "Sounds/vehicle1_s.wav";
    case IDR_VEHICLE2S:    return "Sounds/vehicle2_s.wav";
    case IDR_CRASH_SHORT:  return "Sounds/crashshort.wav";
    case IDR_VEHICLE1E:    return "Sounds/vehicle1_e.wav";
    case IDR_VEHICLE2E:    return "Sounds/vehicle2_e.wav";
    case IDR_CROWD:        return "Sounds/crowd.wav";
    case IDR_OCEAN:        return "Sounds/ocean.wav";
    case IDR_ASPHALT:      return "Sounds/asphalt.wav";
    case IDR_RAIN:         return "Sounds/rain.wav";
    case IDR_WIPERS:       return "Sounds/wipers.wav";
    case IDR_DESERT:       return "Sounds/desert.wav";
    case IDR_AIRPLANE:     return "Sounds/airplane.wav";
    case IDR_CLOCK:        return "Sounds/clock.wav";
    case IDR_VEHICLE1CM:   return "Sounds/vehicle1_cm.wav";
    case IDR_VEHICLE3E:    return "Sounds/vehicle3_e.wav";
    case IDR_VEHICLE4E:    return "Sounds/vehicle4_e.wav";
    case IDR_VEHICLE5E:    return "Sounds/vehicle5_e.wav";
    case IDR_VEHICLE6E:    return "Sounds/vehicle6_e.wav";
    case IDR_VEHICLE7E:    return "Sounds/vehicle7_e.wav";
    case IDR_BUMP1:        return "Sounds/bump.wav";
    case IDR_VEHICLE10H:   return "Sounds/vehicle10_h.wav";
    case IDR_VEHICLE1H:    return "Sounds/vehicle1_h.wav";
    case IDR_VEHICLE2H:    return "Sounds/vehicle2_h.wav";
    case IDR_VEHICLE3H:    return "Sounds/vehicle3_h.wav";
    case IDR_VEHICLE4H:    return "Sounds/vehicle4_h.wav";
    case IDR_VEHICLE5H:    return "Sounds/vehicle5_h.wav";
    case IDR_VEHICLE6H:    return "Sounds/vehicle6_h.wav";
    case IDR_BADSWITCH:    return "Sounds/badswitch.wav";
    case IDR_GRAVEL:       return "Sounds/gravel.wav";
    case IDR_WATER:        return "Sounds/water.wav";
    case IDR_SAND:         return "Sounds/sand.wav";
    case IDR_VEHICLE1T:    return "Sounds/vehicle1_t.wav";
    case IDR_VEHICLE2T:    return "Sounds/vehicle2_t.wav";
    case IDR_VEHICLE9E:    return "Sounds/vehicle9_e.wav";
    case IDR_VEHICLE9S:    return "Sounds/vehicle9_s.wav";
    case IDR_VEHICLE9T:    return "Sounds/vehicle9_t.wav";
    case IDR_VEHICLE9H:    return "Sounds/vehicle9_h.wav";
    case IDR_VEHICLE9F:    return "Sounds/vehicle9_f.wav";
    case IDR_JET:          return "Sounds/jet.wav";
    case IDR_VEHICLE12E:   return "Sounds/vehicle12_e.wav";
    case IDR_VEHICLE12S:   return "Sounds/vehicle12_s.wav";
    case IDR_VEHICLE12H:   return "Sounds/vehicle12_h.wav";
    case IDR_VEHICLE12T:   return "Sounds/vehicle12_t.wav";
    case IDR_VEHICLE12F:   return "Sounds/vehicle12_f.wav";
    case IDR_THUNDER:      return "Sounds/thunder.wav";
    case IDR_VEHICLE3B:    return "Sounds/vehicle3_b.wav";
    case IDR_PILE:         return "Sounds/pile.wav";
    case IDR_VEHICLE8E:    return "Sounds/vehicle8_e.wav";
    case IDR_VEHICLE3C:    return "Sounds/vehicle3_c.wav";
    case IDR_VEHICLE3CM:   return "Sounds/vehicle3_cm.wav";
    case IDR_VEHICLE9C:    return "Sounds/vehicle9_c.wav";
    case IDR_VEHICLE9CM:   return "Sounds/vehicle9_cm.wav";
    case IDR_VEHICLE10C:   return "Sounds/vehicle10_c.wav";
    case IDR_VEHICLE10CM:  return "Sounds/vehicle10_cm.wav";
    case IDR_ERROR:        return "Sounds/error.wav";
    case IDR_SNOW:         return "Sounds/snow.wav";
    case IDR_CONST:        return "Sounds/const.wav";
    case IDR_VEHICLE9B:    return "Sounds/vehicle9_b.wav";
    case IDR_RIVER:        return "Sounds/river.wav";
    case IDR_AIRPORT:      return "Sounds/airport.wav";
    case IDR_WIND:         return "Sounds/wind.wav";
    case IDR_HELICOPTER:   return "Sounds/helicopter.wav";
    case IDR_VEHICLE6B:    return "Sounds/vehicle6_b.wav";
    case IDR_STORM:        return "Sounds/storm.wav";
    case IDR_UNKEY1:       return "Sounds/unkey1.wav";
    case IDR_UNKEY2:       return "Sounds/unkey2.wav";
    case IDR_UNKEY3:       return "Sounds/unkey3.wav";
    case IDR_UNKEY4:       return "Sounds/unkey4.wav";
    case IDR_UNKEY5:       return "Sounds/unkey5.wav";
    case IDR_UNKEY6:       return "Sounds/unkey6.wav";
    case IDR_UNKEY7:       return "Sounds/unkey7.wav";
    case IDR_UNKEY8:       return "Sounds/unkey8.wav";
    case IDR_UNKEY9:       return "Sounds/unkey9.wav";
    case IDR_UNKEY10:      return "Sounds/unkey10.wav";
    case IDR_UNKEY11:      return "Sounds/unkey11.wav";
    case IDR_UNKEY12:      return "Sounds/unkey12.wav";
    case IDR_OWL:          return "Sounds/owl.wav";
    default:
        return nullptr;
    }
}

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
    // Map resource ID to external filename and load from file
    const char* filename = getResourceFilename(resource);
    if (!filename)
    {
        dxTracer.trace("SoundManager::create(resource=%d) - unknown resource ID", resource);
        return nullptr;
    }

    return create(const_cast<Char*>(filename), enable3d, nBuffers);
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
