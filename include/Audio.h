#pragma once

#include <cstdint>

#define ARR_LEN(arr) ((int) (sizeof(arr) / sizeof(*arr)))

struct SoundClip
{
    float* samples;
    int sampleCount;
    uint32_t samplePerSec;
};

struct Sound
{
    double time;
    float amplitude, frequency;
    float amplitudeFactor, frequencyFactor;
    bool looping;
    SoundClip clip;
};

// nice fmt block struct from cute_sound.h
#pragma pack(push, 1)
struct WaveFmtBlock
{
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    // uint16_t cbSize;
    // uint16_t wValidBitsPerSample;
    // uint32_t dwChannelMask;
    // uint8_t  SubFormat[18];
};
#pragma pack(pop) 

namespace Audio
{
	void SetupSound();
	void AudioCallback(float* buffer, int numFrames, int numChannels);

    SoundClip loadSoundClip(const char* filePath);

    Sound* AddSound(float amplitude, float frequency, float amplitudeFactor, float frequencyFactor, bool looping);

    Sound* PlaySoundClip(SoundClip soundClip, float amplitude, float frequency, float amplitudeFactor, float frequencyFactor, bool looping);
    Sound* PlaySound(float amplitude, float frequency, float amplitudeFactor, float frequencyFactor, bool looping);
    void StopSound(Sound& sound);
};