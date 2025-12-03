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
#include <algorithm>
#include <cstring>
#include <cmath>
#include <vector>
#include <mutex>

// Case-insensitive string compare - use strcasecmp on non-Windows
#ifdef _WIN32
#define strcasecmp _stricmp
#endif

// stb_vorbis for OGG file loading
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

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

// =============================================================================
// Resource ID to filename lookup table (from previous implementation)
// =============================================================================

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

// =============================================================================
// AudioChannel - represents one playing sound with pitch/volume/pan control
// =============================================================================

struct AudioChannel
{
    // Source audio data (owned by Sound, not AudioChannel)
    const Uint8* sourceData;
    Uint32 sourceLength;       // Length in bytes
    int sourceFrequency;       // Original sample rate (e.g., 22050)
    int sourceChannels;        // 1 = mono, 2 = stereo
    SDL_AudioFormat sourceFormat;

    // Playback state
    bool active;
    bool looping;
    bool paused;

    // Audio processing - use floating point position for smooth pitch shifting
    double position;           // Current position in SOURCE SAMPLES (not bytes), fractional for interpolation
    double pitchRatio;         // Ratio of playback rate: 1.0 = normal, 2.0 = double speed/pitch
    float volume;              // 0.0 to 1.0
    float pan;                 // -1.0 (left) to +1.0 (right)

    // Owner tracking (for callback when channel finishes)
    DirectX::Sound* owner;

    AudioChannel()
        : sourceData(nullptr)
        , sourceLength(0)
        , sourceFrequency(22050)
        , sourceChannels(2)
        , sourceFormat(AUDIO_S16SYS)
        , active(false)
        , looping(false)
        , paused(false)
        , position(0.0)
        , pitchRatio(1.0)
        , volume(1.0f)
        , pan(0.0f)
        , owner(nullptr)
    {}

    void reset()
    {
        sourceData = nullptr;
        sourceLength = 0;
        position = 0.0;
        pitchRatio = 1.0;
        volume = 1.0f;
        pan = 0.0f;
        active = false;
        looping = false;
        paused = false;
        owner = nullptr;
    }

    // Get total samples in source (per channel)
    int getTotalSamples() const
    {
        int bytesPerSample = (sourceFormat == AUDIO_S16SYS || sourceFormat == AUDIO_S16LSB) ? 2 : 4;
        return sourceLength / (bytesPerSample * sourceChannels);
    }
};

// =============================================================================
// AudioMixer - manages audio device and mixes all active channels
// =============================================================================

class AudioMixer
{
public:
    static const int MAX_CHANNELS = 128;
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
    int allocateChannel(DirectX::Sound* owner, const Uint8* data, Uint32 length,
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
                ch.sourceFrequency = frequency;
                ch.sourceChannels = channels;
                ch.sourceFormat = format;
                ch.position = 0.0;
                // Calculate initial pitch ratio based on source vs device sample rate
                ch.pitchRatio = (double)frequency / (double)m_deviceFrequency;
                ch.active = true;
                ch.owner = owner;

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

    void setChannelVolume(int channel, float volume, DirectX::Sound* owner)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
        AudioChannel& ch = m_channels[channel];
        if (!ch.active || ch.owner != owner)
            return;
        ch.volume = volume;
    }

    void setChannelPan(int channel, float pan, DirectX::Sound* owner)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
        AudioChannel& ch = m_channels[channel];
        if (!ch.active || ch.owner != owner)
            return;
        ch.pan = pan;
    }

    void setChannelFrequency(int channel, int frequency, DirectX::Sound* owner)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;

        std::lock_guard<std::mutex> lock(m_mutex);

        AudioChannel& ch = m_channels[channel];
        if (!ch.active)
            return;

        // Verify ownership - channel may have been reallocated to a different Sound
        if (ch.owner != owner)
            return;

        // Update pitch ratio: frequency is the "perceived" sample rate
        // pitchRatio = how many source samples to advance per device sample
        ch.pitchRatio = (double)frequency / (double)m_deviceFrequency;
    }

    void setChannelLooping(int channel, bool looping)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_channels[channel].looping = looping;
    }

    bool isChannelActive(int channel)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_channels[channel].active;
    }

    void stopChannel(int channel)
    {
        if (channel < 0 || channel >= MAX_CHANNELS)
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
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

// =============================================================================
// Audio callback - called by SDL2 to fill the audio buffer
// Uses linear interpolation for smooth pitch shifting without SDL_AudioStream
// =============================================================================

// Helper: get a sample from source data with linear interpolation
static inline float getSampleInterpolated(const Uint8* data, int totalSamples, int channels,
                                          SDL_AudioFormat format, double position, int channelIndex)
{
    int pos0 = (int)position;
    int pos1 = pos0 + 1;
    double frac = position - pos0;

    // Handle bounds
    if (pos0 < 0) pos0 = 0;
    if (pos1 >= totalSamples) pos1 = totalSamples - 1;
    if (pos0 >= totalSamples) return 0.0f;

    float sample0, sample1;

    if (format == AUDIO_S16SYS || format == AUDIO_S16LSB)
    {
        const Sint16* samples = reinterpret_cast<const Sint16*>(data);
        sample0 = samples[pos0 * channels + channelIndex] / 32768.0f;
        sample1 = samples[pos1 * channels + channelIndex] / 32768.0f;
    }
    else if (format == AUDIO_F32SYS)
    {
        const float* samples = reinterpret_cast<const float*>(data);
        sample0 = samples[pos0 * channels + channelIndex];
        sample1 = samples[pos1 * channels + channelIndex];
    }
    else
    {
        // Unsupported format, return silence
        return 0.0f;
    }

    // Linear interpolation
    return (float)(sample0 + (sample1 - sample0) * frac);
}

void SDLCALL AudioMixer::audioCallback(void* userdata, Uint8* stream, int len)
{
    AudioMixer* mixer = static_cast<AudioMixer*>(userdata);
    float* output = reinterpret_cast<float*>(stream);
    int outputSamples = len / sizeof(float);
    int outputFrames = outputSamples / DEVICE_CHANNELS;  // stereo frames

    // Clear output buffer
    memset(stream, 0, len);

    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        AudioChannel& ch = mixer->m_channels[i];
        if (!ch.active || ch.paused || !ch.sourceData)
            continue;

        int totalSourceSamples = ch.getTotalSamples();
        if (totalSourceSamples <= 0)
        {
            ch.active = false;
            continue;
        }

        // Generate output samples using linear interpolation
        for (int frame = 0; frame < outputFrames; frame++)
        {
            // Check if we've reached the end
            if (ch.position >= totalSourceSamples)
            {
                if (ch.looping)
                {
                    ch.position = fmod(ch.position, (double)totalSourceSamples);
                }
                else
                {
                    ch.active = false;
                    break;
                }
            }

            // Get interpolated samples (handle mono->stereo conversion)
            float left, right;
            if (ch.sourceChannels == 1)
            {
                // Mono source: duplicate to both channels
                left = right = getSampleInterpolated(ch.sourceData, totalSourceSamples,
                                                     ch.sourceChannels, ch.sourceFormat,
                                                     ch.position, 0);
            }
            else
            {
                // Stereo source
                left = getSampleInterpolated(ch.sourceData, totalSourceSamples,
                                            ch.sourceChannels, ch.sourceFormat,
                                            ch.position, 0);
                right = getSampleInterpolated(ch.sourceData, totalSourceSamples,
                                             ch.sourceChannels, ch.sourceFormat,
                                             ch.position, 1);
            }

            // Apply volume with global attenuation to prevent clipping when many sounds play
            const float MASTER_GAIN = 0.4f;  // Reduce overall level to leave headroom for mixing
            left *= ch.volume * MASTER_GAIN;
            right *= ch.volume * MASTER_GAIN;

            // Apply pan using dB-based attenuation (matches DirectSound behavior)
            // DirectSound uses -10000 to +10000 representing -100dB to +100dB attenuation
            // pan: -1.0 (full left) to +1.0 (full right)
            // At full pan, opposite channel is attenuated by 100dB (essentially silent)
            float leftGain, rightGain;
            if (ch.pan > 0.0f)
            {
                // Panning right: attenuate left channel
                float attenuateLeftDb = ch.pan * 100.0f;  // 0 to 100 dB
                leftGain = powf(10.0f, -attenuateLeftDb / 20.0f);
                rightGain = 1.0f;
            }
            else if (ch.pan < 0.0f)
            {
                // Panning left: attenuate right channel
                float attenuateRightDb = -ch.pan * 100.0f;  // 0 to 100 dB
                leftGain = 1.0f;
                rightGain = powf(10.0f, -attenuateRightDb / 20.0f);
            }
            else
            {
                leftGain = 1.0f;
                rightGain = 1.0f;
            }
            left *= leftGain;
            right *= rightGain;

            // Mix into output (additive mixing)
            int outIdx = frame * 2;
            output[outIdx] += left;
            output[outIdx + 1] += right;

            // Advance position by pitch ratio
            ch.position += ch.pitchRatio;
        }
    }

    // Clamp output to prevent clipping
    for (int i = 0; i < outputSamples; i++)
    {
        if (output[i] > 1.0f) output[i] = 1.0f;
        if (output[i] < -1.0f) output[i] = -1.0f;
    }
}

// =============================================================================
// SDL2SoundData - Internal data for each Sound object
// =============================================================================

struct SDL2SoundData
{
    // Raw PCM data (loaded from WAV or OGG)
    Uint8* pcmData;
    Uint32 pcmLength;
    SDL_AudioSpec audioSpec;
    bool fromWav;       // true = use SDL_FreeWAV, false = use SDL_free

    // Playback state
    int channel;        // -1 = not playing
    float volume;       // 0.0 to 1.0 (stored for when not playing)
    float pan;          // -1.0 to +1.0
    int frequency;      // Current target frequency for pitch (may be modified)
    int originalFrequency; // Original sample rate from file (for reset)
    bool is3d;
    float posX, posY, posZ;

    SDL2SoundData()
        : pcmData(nullptr)
        , pcmLength(0)
        , fromWav(true)
        , channel(-1)
        , volume(1.0f)
        , pan(0.0f)
        , frequency(22050)
        , originalFrequency(22050)
        , is3d(false)
        , posX(0), posY(0), posZ(0)
    {
        SDL_zero(audioSpec);
    }

    ~SDL2SoundData()
    {
        if (pcmData)
        {
            if (fromWav)
                SDL_FreeWAV(pcmData);
            else
                SDL_free(pcmData);
            pcmData = nullptr;
        }
    }
};

// Helper to load OGG file using stb_vorbis
static SDL2SoundData* loadOggFile(const char* filename)
{
    int channels, sampleRate;
    short* output;
    int samples = stb_vorbis_decode_filename(filename, &channels, &sampleRate, &output);

    if (samples <= 0)
    {
        dxTracer.trace("Failed to load OGG: %s", filename);
        return nullptr;
    }

    SDL2SoundData* data = new SDL2SoundData();
    data->fromWav = false;  // Use SDL_free, not SDL_FreeWAV

    // stb_vorbis returns 16-bit signed samples
    data->pcmLength = samples * channels * sizeof(short);
    data->pcmData = (Uint8*)SDL_malloc(data->pcmLength);
    if (!data->pcmData)
    {
        free(output);
        delete data;
        return nullptr;
    }
    memcpy(data->pcmData, output, data->pcmLength);
    free(output);

    // Set up audio spec
    data->audioSpec.freq = sampleRate;
    data->audioSpec.format = AUDIO_S16SYS;
    data->audioSpec.channels = (Uint8)channels;
    data->audioSpec.samples = 4096;
    data->audioSpec.size = data->pcmLength;
    data->frequency = sampleRate;
    data->originalFrequency = sampleRate;

    return data;
}

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
    data->originalFrequency = spec.freq;

    return data;
}

// Load audio file - tries WAV first, then OGG
static SDL2SoundData* loadAudioFile(const char* filename)
{
    // Check file extension
    const char* ext = strrchr(filename, '.');

    if (ext && (strcasecmp(ext, ".ogg") == 0))
    {
        // OGG file - use stb_vorbis
        return loadOggFile(filename);
    }
    else if (ext && (strcasecmp(ext, ".wav") == 0))
    {
        // WAV file - use SDL_LoadWAV
        return loadWavFile(filename);
    }
    else
    {
        // Unknown extension - try WAV first, then OGG
        SDL2SoundData* data = loadWavFile(filename);
        if (!data)
            data = loadOggFile(filename);
        return data;
    }
}

namespace DirectX
{

// Helper to get SDL2 data from Sound (uses DirectX::Sound)
static SDL2SoundData* GetSDL2Data(Sound* sound)
{
    if (!sound || !sound->buffer())
        return nullptr;
    return reinterpret_cast<SDL2SoundData*>(sound->buffer()[0]);
}

//-----------------------------------------------------------------------------
// SoundManager Implementation
//-----------------------------------------------------------------------------

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

    SDL2SoundData* data = loadAudioFile(filename);
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

//-----------------------------------------------------------------------------
// Sound Implementation - Playback Methods
//-----------------------------------------------------------------------------

Int Sound::play(UInt priority, Boolean looped)
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || !data->pcmData)
    {
        dxTracer.trace("Sound::play FAILED - no data or pcmData");
        return dxFailed;
    }

    // Check if our stored channel is still valid and owned by us
    if (data->channel >= 0)
    {
        AudioChannel* ch = AudioMixer::instance()->getChannel(data->channel);
        if (ch && ch->active && ch->owner == this)
        {
            // Sound is still playing on this channel
            if (looped)
            {
                // For looped sounds, restart from beginning
                dxTracer.trace("Sound::play - stopping looped sound to restart");
                stop();
            }
            // For non-looped, allocate a new channel (allows overlapping playback)
            // Don't stop the existing one - let it finish naturally
        }
        else
        {
            // Channel finished or was reused by another sound - clear our stale reference
            data->channel = -1;
        }
    }

    // Allocate a mixer channel
    int oldChannel = data->channel;
    data->channel = AudioMixer::instance()->allocateChannel(
        this,
        data->pcmData,
        data->pcmLength,
        data->frequency,
        data->audioSpec.channels,
        data->audioSpec.format
    );

    if (data->channel < 0)
    {
        dxTracer.trace("Sound::play FAILED - no free channel (was ch %d)", oldChannel);
        return dxFailed;
    }

    dxTracer.trace("Sound::play OK - allocated ch %d, len=%u, freq=%d, vol=%.2f",
        data->channel, data->pcmLength, data->frequency, data->volume);

    // Apply stored settings
    AudioMixer::instance()->setChannelVolume(data->channel, data->volume, this);
    AudioMixer::instance()->setChannelPan(data->channel, data->pan, this);
    AudioMixer::instance()->setChannelLooping(data->channel, looped != FALSE);

    return dxSuccess;
}

Int Sound::stop()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return dxSuccess;

    // Verify we still own this channel before stopping it
    // (channel may have been reused by another sound after we finished)
    AudioChannel* ch = AudioMixer::instance()->getChannel(data->channel);
    if (ch && ch->owner == this)
    {
        dxTracer.trace("Sound::stop - freeing ch %d (active=%d)", data->channel, ch->active ? 1 : 0);
        AudioMixer::instance()->freeChannel(data->channel);
    }
    else
    {
        dxTracer.trace("Sound::stop - ch %d not owned by us, skipping", data->channel);
    }
    data->channel = -1;
    return dxSuccess;
}

Int Sound::reset()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (data)
    {
        // Reset frequency to original sample rate
        data->frequency = data->originalFrequency;
    }
    return stop();
}

Boolean Sound::playing()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data || data->channel < 0)
        return false;

    // Verify we still own this channel (it may have been reused)
    AudioChannel* ch = AudioMixer::instance()->getChannel(data->channel);
    if (!ch || ch->owner != this)
        return false;

    return ch->active;
}

//-----------------------------------------------------------------------------
// Sound Implementation - Control Methods
//-----------------------------------------------------------------------------

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
        AudioMixer::instance()->setChannelPan(data->channel, pan, this);
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
        AudioMixer::instance()->setChannelFrequency(data->channel, value, this);
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
        AudioMixer::instance()->setChannelVolume(data->channel, vol, this);
}

Int Sound::volume()
{
    SDL2SoundData* data = GetSDL2Data(this);
    if (!data)
        return 0;
    return static_cast<Int>(data->volume * 100.0f);
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
        AudioMixer::instance()->setChannelPan(data->channel, pan, this);
        AudioMixer::instance()->setChannelVolume(data->channel, vol, this);
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
