# SDL2 Audio Mixer with Pitch Control Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace SDL2_mixer with a custom SDL2 low-level audio mixer that supports real-time pitch control via resampling, enabling engine sounds to vary with RPM.

**Architecture:** Custom audio mixer using SDL2's audio callback API with SDL_AudioStream for per-channel resampling. Each Sound object maintains its own AudioStream that can change pitch ratio in real-time. A central mixer combines all active channels in the audio callback.

**Tech Stack:** SDL2 audio API (SDL_OpenAudioDevice, SDL_AudioStream), C++, existing DxCommon/Sound interface

---

## Background

The game uses DirectSound's `SetFrequency()` for pitch control:
- Engine sounds: 6000-60000 Hz (0.27x to 2.7x pitch ratio)
- Surface sounds: up to 100000 Hz (4.5x pitch ratio)
- Brake sounds: 11025-33075 Hz range

SDL2_mixer doesn't support real-time pitch. SDL2's `SDL_AudioStream` provides resampling that we can use for pitch shifting.

## File Overview

| File | Action | Purpose |
|------|--------|---------|
| `vs_projects/dxcommon/Src/Sound_SDL2.cpp` | Replace | New mixer implementation |
| `vs_projects/dxcommon/If/Sound.h` | Modify | Add SDL2 mixer types |
| `vs_projects/dxcommon/If/SDL2Compat.h` | Modify | Remove SDL_mixer include |
| `vs_projects/dxcommon/CMakeLists.txt` | Modify | Remove SDL2_mixer dependency |

---

### Task 1: Create AudioMixer Singleton Structure

**Files:**
- Modify: `vs_projects/dxcommon/If/Sound.h:12-22`
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp:1-250`

**Step 1: Add forward declarations and mixer class to Sound.h**

In `vs_projects/dxcommon/If/Sound.h`, after line 22 (after the `#endif` for SDL2 includes), add:

```cpp
#ifdef TOPSPEED_USE_SDL2
// Forward declarations for SDL2 audio mixer
class AudioMixer;
struct AudioChannel;
#endif
```

**Step 2: Create AudioMixer class skeleton in Sound_SDL2.cpp**

Replace the top section of `vs_projects/dxcommon/Src/Sound_SDL2.cpp` (lines 1-250) with:

```cpp
/**
 * DXCommon library - SDL2 Sound Implementation
 * Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
 * SDL2 port 2025
 *
 * Custom audio mixer with pitch control via SDL_AudioStream resampling.
 */
#ifdef TOPSPEED_USE_SDL2

#include <DxCommon/If/Sound.h>
#include <Common/If/Tracer.h>
#include <SDL.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <mutex>

extern Tracer dxTracer;

// =============================================================================
// Resource ID to filename lookup table (from previous implementation)
// =============================================================================

// [Keep all the IDR_* defines and getResourceFilename() function - lines 16-195 unchanged]
```

**Step 3: Build and verify compilation**

Run: `cmake --build build --config Debug 2>&1 | head -50`
Expected: Build succeeds (no new code yet, just reorganization)

**Step 4: Commit**

```bash
git add vs_projects/dxcommon/If/Sound.h vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "refactor: prepare Sound_SDL2.cpp for custom mixer"
```

---

### Task 2: Implement AudioChannel Structure

**Files:**
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Add AudioChannel struct after the resource lookup table**

Add after `getResourceFilename()` function (~line 196):

```cpp
// =============================================================================
// AudioChannel - represents one playing sound with pitch/volume/pan control
// =============================================================================

struct AudioChannel
{
    // Source audio data (owned by Sound, not AudioChannel)
    const Uint8* sourceData;
    Uint32 sourceLength;
    Uint32 sourcePosition;
    int sourceFrequency;     // Original sample rate (e.g., 22050)
    int sourceChannels;      // 1 = mono, 2 = stereo
    SDL_AudioFormat sourceFormat;

    // Playback state
    bool active;
    bool looping;
    bool paused;

    // Audio processing
    SDL_AudioStream* stream;
    int targetFrequency;     // Current pitch target (e.g., 44100 for 2x pitch)
    float volume;            // 0.0 to 1.0
    float pan;               // -1.0 (left) to +1.0 (right)

    // Owner tracking (for callback when channel finishes)
    class Sound* owner;

    AudioChannel()
        : sourceData(nullptr)
        , sourceLength(0)
        , sourcePosition(0)
        , sourceFrequency(22050)
        , sourceChannels(2)
        , sourceFormat(AUDIO_S16SYS)
        , active(false)
        , looping(false)
        , paused(false)
        , stream(nullptr)
        , targetFrequency(22050)
        , volume(1.0f)
        , pan(0.0f)
        , owner(nullptr)
    {}

    void reset()
    {
        if (stream)
        {
            SDL_FreeAudioStream(stream);
            stream = nullptr;
        }
        sourceData = nullptr;
        sourceLength = 0;
        sourcePosition = 0;
        active = false;
        looping = false;
        paused = false;
        owner = nullptr;
    }
};
```

**Step 2: Build and verify compilation**

Run: `cmake --build build --config Debug 2>&1 | head -30`
Expected: Build succeeds

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "feat: add AudioChannel structure for SDL2 mixer"
```

---

### Task 3: Implement AudioMixer Class

**Files:**
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Add AudioMixer class after AudioChannel**

```cpp
// =============================================================================
// AudioMixer - manages audio device and mixes all active channels
// =============================================================================

class AudioMixer
{
public:
    static const int MAX_CHANNELS = 64;
    static const int DEVICE_FREQUENCY = 44100;
    static const int DEVICE_CHANNELS = 2;
    static const int DEVICE_SAMPLES = 1024;

    static AudioMixer* instance()
    {
        static AudioMixer s_instance;
        return &s_instance;
    }

    bool initialize()
    {
        if (m_initialized)
            return true;

        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
        {
            dxTracer.trace("AudioMixer: SDL_InitSubSystem(AUDIO) failed: %s", SDL_GetError());
            return false;
        }

        SDL_AudioSpec desired, obtained;
        SDL_zero(desired);
        desired.freq = DEVICE_FREQUENCY;
        desired.format = AUDIO_F32SYS;
        desired.channels = DEVICE_CHANNELS;
        desired.samples = DEVICE_SAMPLES;
        desired.callback = audioCallback;
        desired.userdata = this;

        m_deviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
        if (m_deviceId == 0)
        {
            dxTracer.trace("AudioMixer: SDL_OpenAudioDevice failed: %s", SDL_GetError());
            return false;
        }

        m_deviceFrequency = obtained.freq;
        m_deviceChannels = obtained.channels;
        m_deviceFormat = obtained.format;

        // Unpause the audio device to start playback
        SDL_PauseAudioDevice(m_deviceId, 0);

        m_initialized = true;
        dxTracer.trace("AudioMixer: Initialized (freq=%d, channels=%d)", m_deviceFrequency, m_deviceChannels);
        return true;
    }

    void shutdown()
    {
        if (!m_initialized)
            return;

        SDL_CloseAudioDevice(m_deviceId);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);

        for (int i = 0; i < MAX_CHANNELS; i++)
            m_channels[i].reset();

        m_initialized = false;
    }

    // Allocate a channel for a sound
    int allocateChannel(Sound* owner, const Uint8* data, Uint32 length,
                        int frequency, int channels, SDL_AudioFormat format)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            if (!m_channels[i].active)
            {
                AudioChannel& ch = m_channels[i];
                ch.reset();
                ch.sourceData = data;
                ch.sourceLength = length;
                ch.sourcePosition = 0;
                ch.sourceFrequency = frequency;
                ch.sourceChannels = channels;
                ch.sourceFormat = format;
                ch.targetFrequency = frequency;
                ch.active = true;
                ch.owner = owner;

                // Create audio stream for resampling
                ch.stream = SDL_NewAudioStream(
                    format, channels, frequency,
                    m_deviceFormat, m_deviceChannels, m_deviceFrequency
                );

                if (!ch.stream)
                {
                    dxTracer.trace("AudioMixer: Failed to create AudioStream: %s", SDL_GetError());
                    ch.active = false;
                    return -1;
                }

                return i;
            }
        }

        dxTracer.trace("AudioMixer: No free channels available");
        return -1;
    }

    void freeChannel(int channel)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_channels[channel].reset();
    }

    void setChannelVolume(int channel, float volume)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        m_channels[channel].volume = volume;
    }

    void setChannelPan(int channel, float pan)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        m_channels[channel].pan = pan;
    }

    void setChannelFrequency(int channel, int frequency)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;

        AudioChannel& ch = m_channels[channel];
        if (!ch.active || !ch.stream)
            return;

        if (frequency == ch.targetFrequency)
            return;

        ch.targetFrequency = frequency;

        // Recreate the audio stream with new frequency ratio
        // SDL_AudioStream resamples from source to device rate
        // To change pitch, we change the "source" rate
        SDL_FreeAudioStream(ch.stream);
        ch.stream = SDL_NewAudioStream(
            ch.sourceFormat, ch.sourceChannels, frequency,
            m_deviceFormat, m_deviceChannels, m_deviceFrequency
        );
    }

    void setChannelLooping(int channel, bool looping)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        m_channels[channel].looping = looping;
    }

    bool isChannelActive(int channel)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return false;
        return m_channels[channel].active;
    }

    void stopChannel(int channel)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        m_channels[channel].active = false;
    }

    AudioChannel* getChannel(int channel)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return nullptr;
        return &m_channels[channel];
    }

private:
    AudioMixer()
        : m_initialized(false)
        , m_deviceId(0)
        , m_deviceFrequency(DEVICE_FREQUENCY)
        , m_deviceChannels(DEVICE_CHANNELS)
        , m_deviceFormat(AUDIO_F32SYS)
    {}

    ~AudioMixer()
    {
        shutdown();
    }

    static void SDLCALL audioCallback(void* userdata, Uint8* stream, int len);

    bool m_initialized;
    SDL_AudioDeviceID m_deviceId;
    int m_deviceFrequency;
    int m_deviceChannels;
    SDL_AudioFormat m_deviceFormat;
    AudioChannel m_channels[MAX_CHANNELS];
    std::mutex m_mutex;
};
```

**Step 2: Build and verify compilation**

Run: `cmake --build build --config Debug 2>&1 | head -30`
Expected: Build succeeds (linker error for audioCallback is OK, we'll implement it next)

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "feat: add AudioMixer class for SDL2 audio"
```

---

### Task 4: Implement Audio Callback (Mixing Logic)

**Files:**
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Add the audio callback implementation after AudioMixer class**

```cpp
// =============================================================================
// Audio callback - called by SDL2 to fill the audio buffer
// =============================================================================

void SDLCALL AudioMixer::audioCallback(void* userdata, Uint8* stream, int len)
{
    AudioMixer* mixer = static_cast<AudioMixer*>(userdata);
    float* output = reinterpret_cast<float*>(stream);
    int samples = len / sizeof(float);

    // Clear output buffer
    memset(stream, 0, len);

    // Temporary buffer for channel audio
    static float tempBuffer[4096];

    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        AudioChannel& ch = mixer->m_channels[i];
        if (!ch.active || ch.paused || !ch.stream)
            continue;

        // Feed source data to the stream if needed
        int available = SDL_AudioStreamAvailable(ch.stream);
        int needed = len * 2;  // Request more than we need to keep stream fed

        while (available < needed && ch.sourcePosition < ch.sourceLength)
        {
            // Calculate how much source data to feed
            int bytesToFeed = ch.sourceLength - ch.sourcePosition;
            if (bytesToFeed > 4096)
                bytesToFeed = 4096;

            int result = SDL_AudioStreamPut(ch.stream,
                ch.sourceData + ch.sourcePosition, bytesToFeed);

            if (result < 0)
            {
                dxTracer.trace("AudioMixer: SDL_AudioStreamPut failed: %s", SDL_GetError());
                break;
            }

            ch.sourcePosition += bytesToFeed;
            available = SDL_AudioStreamAvailable(ch.stream);

            // Handle looping
            if (ch.sourcePosition >= ch.sourceLength)
            {
                if (ch.looping)
                {
                    ch.sourcePosition = 0;
                }
                else
                {
                    // Signal end of stream
                    SDL_AudioStreamFlush(ch.stream);
                    break;
                }
            }
        }

        // Get resampled audio from stream
        int got = SDL_AudioStreamGet(ch.stream, tempBuffer, len);
        if (got <= 0)
        {
            // No more audio available
            if (ch.sourcePosition >= ch.sourceLength && !ch.looping)
            {
                ch.active = false;
            }
            continue;
        }

        int gotSamples = got / sizeof(float);

        // Apply volume and pan, mix into output
        for (int j = 0; j < gotSamples; j += 2)
        {
            float left = tempBuffer[j];
            float right = (j + 1 < gotSamples) ? tempBuffer[j + 1] : left;

            // Apply volume
            left *= ch.volume;
            right *= ch.volume;

            // Apply pan (-1 = full left, +1 = full right)
            float leftGain = (ch.pan <= 0) ? 1.0f : (1.0f - ch.pan);
            float rightGain = (ch.pan >= 0) ? 1.0f : (1.0f + ch.pan);
            left *= leftGain;
            right *= rightGain;

            // Mix into output (additive mixing)
            if (j < samples)
                output[j] += left;
            if (j + 1 < samples)
                output[j + 1] += right;
        }
    }

    // Clamp output to prevent clipping
    for (int i = 0; i < samples; i++)
    {
        if (output[i] > 1.0f) output[i] = 1.0f;
        if (output[i] < -1.0f) output[i] = -1.0f;
    }
}
```

**Step 2: Build and verify compilation**

Run: `cmake --build build --config Debug 2>&1 | head -30`
Expected: Build succeeds

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "feat: implement audio mixing callback with pitch/volume/pan"
```

---

### Task 5: Implement WAV Loading (Replace Mix_LoadWAV)

**Files:**
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Add WAV loading helper and SDL2SoundData structure**

Replace the old `SDL2SoundData` struct with:

```cpp
// =============================================================================
// SDL2SoundData - Internal data for each Sound object
// =============================================================================

struct SDL2SoundData
{
    // Raw PCM data (loaded from WAV)
    Uint8* pcmData;
    Uint32 pcmLength;
    SDL_AudioSpec audioSpec;

    // Playback state
    int channel;        // -1 = not playing
    float volume;       // 0.0 to 1.0 (stored for when not playing)
    float pan;          // -1.0 to +1.0
    int frequency;      // Target frequency for pitch
    bool is3d;
    float posX, posY, posZ;

    SDL2SoundData()
        : pcmData(nullptr)
        , pcmLength(0)
        , channel(-1)
        , volume(1.0f)
        , pan(0.0f)
        , frequency(22050)
        , is3d(false)
        , posX(0), posY(0), posZ(0)
    {
        SDL_zero(audioSpec);
    }

    ~SDL2SoundData()
    {
        if (pcmData)
        {
            SDL_FreeWAV(pcmData);
            pcmData = nullptr;
        }
    }
};

// Helper to load WAV file
static SDL2SoundData* loadWavFile(const char* filename)
{
    SDL_AudioSpec spec;
    Uint8* buffer = nullptr;
    Uint32 length = 0;

    if (!SDL_LoadWAV(filename, &spec, &buffer, &length))
    {
        dxTracer.trace("Failed to load WAV: %s - %s", filename, SDL_GetError());
        return nullptr;
    }

    SDL2SoundData* data = new SDL2SoundData();
    data->pcmData = buffer;
    data->pcmLength = length;
    data->audioSpec = spec;
    data->frequency = spec.freq;

    return data;
}
```

**Step 2: Build and verify compilation**

Run: `cmake --build build --config Debug`
Expected: Build succeeds

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "feat: add WAV loading for SDL2 mixer"
```

---

### Task 6: Rewrite SoundManager to Use AudioMixer

**Files:**
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Replace SoundManager implementation**

Replace the existing SoundManager implementation (after the resource lookup) with:

```cpp
namespace DirectX
{

// =============================================================================
// SoundManager Implementation
// =============================================================================

SoundManager::SoundManager(::Window::Handle hwnd, UInt nChannels, UInt frequency, UInt bitrate)
    : m_directSound(nullptr)
    , m_created(false)
    , m_playInSoftware(false)
    , m_reverseStereo(false)
    , m_3dAlgorithm(AlgoDefault)
{
    m_created = AudioMixer::instance()->initialize();
    if (m_created)
        dxTracer.trace("SoundManager: AudioMixer initialized");
    else
        dxTracer.trace("SoundManager: AudioMixer initialization failed");
}

SoundManager::~SoundManager()
{
    AudioMixer::instance()->shutdown();
}

Sound* SoundManager::create(Int resource, Boolean enable3d, UInt nBuffers)
{
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

    SDL2SoundData* data = loadWavFile(filename);
    if (!data)
        return nullptr;

    data->is3d = enable3d;

    Sound* sound = new Sound(data);
    sound->reverseStereo(m_reverseStereo);
    dxTracer.trace("Loaded sound: %s (freq=%d, ch=%d)",
        filename, data->audioSpec.freq, data->audioSpec.channels);
    return sound;
}

Sound* SoundManager::create(DSBUFFERDESC& bufferDesc, Boolean enable3d, UInt nBuffers)
{
    dxTracer.trace("SoundManager::create(bufferDesc) - not implemented");
    return nullptr;
}

#ifdef _USE_VORBIS_
Sound* SoundManager::createVorbis(Char* filename, Boolean enable3d, UInt nBuffers)
{
    // SDL_LoadWAV doesn't handle OGG, would need separate implementation
    // For now, try loading as WAV (some .ogg files might actually be .wav)
    return create(filename, enable3d, nBuffers);
}
#endif

Int SoundManager::bufferFormat(UInt nChannels, UInt frequency, UInt bitrate)
{
    return dxSuccess;
}

Int SoundManager::listener3DInterface(LPDIRECTSOUND3DLISTENER* listener)
{
    if (listener)
        *listener = nullptr;
    return dxSuccess;
}
```

**Step 2: Build and verify compilation**

Run: `cmake --build build --config Debug`
Expected: May have errors due to Sound constructor change - we'll fix in next task

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "feat: rewrite SoundManager to use AudioMixer"
```

---

### Task 7: Rewrite Sound Class Implementation

**Files:**
- Modify: `vs_projects/dxcommon/If/Sound.h:113-117`
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Update Sound.h to add SDL2SoundData constructor**

In `vs_projects/dxcommon/If/Sound.h`, replace lines 113-117:

```cpp
#ifdef TOPSPEED_USE_SDL2
    // SDL2-specific constructor
    _dxcommon_ Sound(struct SDL2SoundData* data);
#endif
```

**Step 2: Implement Sound class methods**

Replace the Sound implementation section with:

```cpp
// =============================================================================
// Sound Implementation
// =============================================================================

// Helper to get SDL2 data from Sound
static SDL2SoundData* GetSDL2Data(Sound* sound)
{
    if (!sound || !sound->buffer())
        return nullptr;
    return reinterpret_cast<SDL2SoundData*>(sound->buffer()[0]);
}

// DirectX compatibility constructors (not used in SDL2 mode)
Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers, WaveFile* waveFile)
    : m_buffer(nullptr), m_bufferSize(bufferSize), m_nBuffers(nBuffers)
    , m_waveFile(waveFile), m_playInSoftware(false), m_reverseStereo(1)
    , m_buffer3D(nullptr), m_length(0.0f)
{}

Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers, LPWAVEFORMATEX waveFormat)
    : m_buffer(nullptr), m_bufferSize(bufferSize), m_nBuffers(nBuffers)
    , m_waveFile(nullptr), m_playInSoftware(false), m_reverseStereo(1)
    , m_buffer3D(nullptr), m_length(0.0f)
{}

#ifdef _USE_VORBIS_
Sound::Sound(LPDIRECTSOUNDBUFFER* buffer, UInt bufferSize, UInt nBuffers,
             OggVorbis_File* vorbisFile, UShort bitsPerSample, UInt avgBytesPerSec)
    : m_buffer(nullptr), m_bufferSize(bufferSize), m_nBuffers(nBuffers)
    , m_waveFile(nullptr), m_playInSoftware(false), m_reverseStereo(1)
    , m_buffer3D(nullptr), m_length(0.0f)
{}
#endif

// SDL2-specific constructor
Sound::Sound(SDL2SoundData* data)
    : m_buffer(new LPDIRECTSOUNDBUFFER[1])
    , m_bufferSize(data ? data->pcmLength : 0)
    , m_nBuffers(1)
    , m_waveFile(nullptr)
    , m_playInSoftware(false)
    , m_reverseStereo(1)
    , m_buffer3D(nullptr)
    , m_length(0.0f)
{
    m_buffer[0] = reinterpret_cast<LPDIRECTSOUNDBUFFER>(data);

    if (data && data->pcmLength > 0 && data->audioSpec.freq > 0)
    {
        int bytesPerSample = SDL_AUDIO_BITSIZE(data->audioSpec.format) / 8;
        int bytesPerSecond = data->audioSpec.freq * data->audioSpec.channels * bytesPerSample;
        if (bytesPerSecond > 0)
            m_length = static_cast<float>(data->pcmLength) / bytesPerSecond;
    }
}

Sound::~Sound()
{
    stop();

    if (m_buffer)
    {
        SDL2SoundData* data = GetSDL2Data(this);
        delete data;  // SDL2SoundData destructor frees PCM data
        delete[] m_buffer;
        m_buffer = nullptr;
    }
}

Int Sound::play(UInt priority, Boolean looped)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || !data->pcmData)
        return dxFailed;

    // Stop any existing playback
    if (data->channel >= 0)
        stop();

    // Allocate a mixer channel
    data->channel = AudioMixer::instance()->allocateChannel(
        this,
        data->pcmData,
        data->pcmLength,
        data->frequency,
        data->audioSpec.channels,
        data->audioSpec.format
    );

    if (data->channel < 0)
        return dxFailed;

    // Apply stored settings
    AudioMixer::instance()->setChannelVolume(data->channel, data->volume);
    AudioMixer::instance()->setChannelPan(data->channel, data->pan);
    AudioMixer::instance()->setChannelLooping(data->channel, looped != FALSE);

    return dxSuccess;
}

Int Sound::stop()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return dxSuccess;

    AudioMixer::instance()->freeChannel(data->channel);
    data->channel = -1;
    return dxSuccess;
}

Int Sound::reset()
{
    return stop();
}

Boolean Sound::playing()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return false;
    return AudioMixer::instance()->isChannelActive(data->channel);
}

void Sound::pan(Int value)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return;

    // Convert -100..+100 to -1.0..+1.0
    float pan = static_cast<float>(value) / 100.0f;
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;

    // Apply reverse stereo if needed
    if (m_reverseStereo < 0)
        pan = -pan;

    data->pan = pan;

    if (data->channel >= 0)
        AudioMixer::instance()->setChannelPan(data->channel, pan);
}

void Sound::frequency(Int value)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return;

    if (value < 100) value = 100;      // Minimum frequency
    if (value > 200000) value = 200000; // Maximum frequency

    data->frequency = value;

    if (data->channel >= 0)
        AudioMixer::instance()->setChannelFrequency(data->channel, value);
}

Int Sound::frequency()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return 22050;
    return data->frequency;
}

void Sound::volume(Int value)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return;

    // Convert 0-100 to 0.0-1.0
    float vol = static_cast<float>(value) / 100.0f;
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;

    data->volume = vol;

    if (data->channel >= 0)
        AudioMixer::instance()->setChannelVolume(data->channel, vol);
}

Int Sound::volume()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return 0;
    return static_cast<Int>(data->volume * 100.0f);
}

Int Sound::initializeBuffer3D(UInt index)
{
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

    // Simple 3D: pan based on X, volume based on distance
    float pan = pos.x * 0.1f;
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;
    data->pan = pan;

    float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    float vol = data->volume;
    if (dist > 1.0f)
        vol = data->volume / dist;

    if (data->channel >= 0)
    {
        AudioMixer::instance()->setChannelPan(data->channel, pan);
        AudioMixer::instance()->setChannelVolume(data->channel, vol);
    }
}
```

**Step 3: Build and verify compilation**

Run: `cmake --build build --config Debug`
Expected: Build succeeds

**Step 4: Commit**

```bash
git add vs_projects/dxcommon/If/Sound.h vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "feat: rewrite Sound class to use AudioMixer with pitch control"
```

---

### Task 8: Keep Remaining Stub Implementations

**Files:**
- Modify: `vs_projects/dxcommon/Src/Sound_SDL2.cpp`

**Step 1: Add remaining stub implementations (unchanged from before)**

```cpp
// =============================================================================
// Sound Implementation - Low-level Methods (stubs)
// =============================================================================

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

// =============================================================================
// WaveFile Implementation (stub - not used with SDL2)
// =============================================================================

WaveFile::WaveFile()
    : m_waveFormat(nullptr), m_mmioHandle(nullptr), m_size(0)
    , m_flags(0), m_fromMemory(false), m_data(nullptr)
    , m_dataCurrent(nullptr), m_dataSize(0), m_buffer(nullptr)
{
    memset(&m_ckInfo, 0, sizeof(m_ckInfo));
    memset(&m_ckInfoRiff, 0, sizeof(m_ckInfoRiff));
    memset(&m_mmioInfo, 0, sizeof(m_mmioInfo));
}

WaveFile::~WaveFile() { close(); }
Int WaveFile::open(Char* filename, WAVEFORMATEX* format, UInt flags) { return dxFailed; }
Int WaveFile::openFromMemory(UByte* buffer, UInt bufferSize, WAVEFORMATEX* format, UInt flags) { return dxFailed; }
Int WaveFile::close() { return dxSuccess; }
Int WaveFile::read(UByte* pBuffer, UInt dwSizeToRead, UInt* pdwSizeRead) { if (pdwSizeRead) *pdwSizeRead = 0; return dxFailed; }
Int WaveFile::write(UInt nSizeToWrite, UByte* pbData, UInt* pnSizeWrote) { if (pnSizeWrote) *pnSizeWrote = 0; return dxFailed; }
Int WaveFile::resetFile() { return dxSuccess; }
Int WaveFile::readMmio() { return dxFailed; }
Int WaveFile::writeMmio(WAVEFORMATEX* pwfxDest) { return dxFailed; }

// =============================================================================
// Listener3D Implementation (stub)
// =============================================================================

Listener3D::Listener3D(SoundManager* soundManager) : m_ds3DListener(nullptr)
{
    memset(&m_parameters, 0, sizeof(m_parameters));
}

Listener3D::~Listener3D() {}
void Listener3D::parameters(Float dopplerFactor, Float rolloffFactor) {}
void Listener3D::position(Vector3 pos) {}
void Listener3D::orientation(Float dir) {}
void Listener3D::positionAndOrientation(Vector3 pos, Float dir) {}

} // namespace DirectX

#endif // TOPSPEED_USE_SDL2
```

**Step 2: Build and verify compilation**

Run: `cmake --build build --config Debug`
Expected: Build succeeds

**Step 3: Commit**

```bash
git add vs_projects/dxcommon/Src/Sound_SDL2.cpp
git commit -m "feat: add remaining Sound stubs for SDL2 mixer"
```

---

### Task 9: Remove SDL2_mixer Dependency

**Files:**
- Modify: `vs_projects/dxcommon/If/SDL2Compat.h:19`
- Modify: `vs_projects/dxcommon/If/Sound.h:15`
- Modify: `vs_projects/dxcommon/CMakeLists.txt:50`

**Step 1: Remove SDL_mixer.h include from SDL2Compat.h**

In `vs_projects/dxcommon/If/SDL2Compat.h`, remove line 19:
```cpp
#include <SDL_mixer.h>
```

**Step 2: Remove SDL_mixer.h include from Sound.h**

In `vs_projects/dxcommon/If/Sound.h`, remove line 15:
```cpp
#include <SDL_mixer.h>
```

Also remove lines 16-17:
```cpp
// Forward declare Mix_Chunk for SDL2 Sound constructor
struct Mix_Chunk;
```

**Step 3: Remove SDL2_mixer from CMakeLists.txt**

In `vs_projects/dxcommon/CMakeLists.txt`, remove line 50:
```cmake
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
```

**Step 4: Build and verify compilation**

Run: `cmake --build build --config Debug`
Expected: Build succeeds without SDL2_mixer

**Step 5: Commit**

```bash
git add vs_projects/dxcommon/If/SDL2Compat.h vs_projects/dxcommon/If/Sound.h vs_projects/dxcommon/CMakeLists.txt
git commit -m "refactor: remove SDL2_mixer dependency, use custom mixer"
```

---

### Task 10: Test the Implementation

**Files:**
- None (testing only)

**Step 1: Rebuild the entire project**

Run:
```bash
cd F:/src/TopSpeed
cmake --build build --config Debug --clean-first
```
Expected: Build succeeds

**Step 2: Run the game and test audio**

Run: `cd installer/tspeed && ./TopSpeed.exe`

Test checklist:
- [ ] Logo sound plays
- [ ] Menu navigation sounds work
- [ ] Start a race - engine sound should vary pitch with speed
- [ ] Surface sounds should vary pitch with speed
- [ ] Verify no crashes or audio glitches

**Step 3: Check trace log for errors**

Run: `cat installer/tspeed/TopSpeed.trc | grep -i "audio\|sound\|mixer"`
Expected: Initialization messages, no errors

**Step 4: Commit final state**

```bash
git add -A
git commit -m "test: verify SDL2 audio mixer with pitch control"
```

---

## Execution Summary

| Task | Description | Est. Time |
|------|-------------|-----------|
| 1 | Create AudioMixer structure | 5 min |
| 2 | Implement AudioChannel | 5 min |
| 3 | Implement AudioMixer class | 10 min |
| 4 | Implement audio callback | 10 min |
| 5 | Implement WAV loading | 5 min |
| 6 | Rewrite SoundManager | 5 min |
| 7 | Rewrite Sound class | 10 min |
| 8 | Keep remaining stubs | 3 min |
| 9 | Remove SDL2_mixer dependency | 3 min |
| 10 | Test implementation | 10 min |

**Total: ~66 minutes**

---

## Rollback Plan

If issues arise, the original `Sound_SDL2.cpp` can be restored:
```bash
git checkout HEAD~N -- vs_projects/dxcommon/Src/Sound_SDL2.cpp
```

The SDL2_mixer dependency can be re-added to CMakeLists.txt if needed.
