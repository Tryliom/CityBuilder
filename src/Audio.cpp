#include "Audio.h"

#include <cassert>

#define SOKOL_AUDIO_IMPL
#include "sokol_audio.h"
#include "sokol_log.h"

#include "Constants.h"
#include "FileSystem.h"
#include "Maths.h"

Sound playingSounds[10];
int playingSoundsCount = 0;

namespace Audio
{
	void AudioCallback(float* buffer, int numFrames, int numChannels)
	{
		for (int frame = 0; frame < numFrames; frame++)
		{
			buffer[frame] = 0;

			for (int i = 0; i < playingSoundsCount; i++)
			{
				Sound* sound = &playingSounds[i];

				sound->amplitude -= sound->amplitudeFactor;
				sound->amplitude = MAX(0, sound->amplitude);

				sound->frequency -= sound->frequencyFactor;
				sound->frequency = MAX(0, sound->frequency);

				bool soundFinished = false;

				if (sound->amplitude <= 0)
				{
					soundFinished = true;
				}

				if (sound->clip.samples)
				{
					buffer[frame] += sound->clip.samples[(int)sound->time] * 0.2f * sound->amplitude;

					sound->time += 1.0 / 44100 * sound->clip.samplePerSec;

					if (sound->time >= sound->clip.sampleCount)
					{
						soundFinished = true;
						printf("sound finished");
					}
				}
				else
				{
					// 2 * PI = un tour de cercle, donc une oscillation de sin
					buffer[frame] += sinf(sound->time * MathUtility::TAU) * 0.2f * sound->amplitude; // * 0.n -> baisser le son.

					// 44100 = samples per second
					sound->time += 1.0 / 44100 * sound->frequency; // time between 2 sample
				}

				if (soundFinished)
				{
					if (sound->looping)
					{
						sound->time = 0;
					}
					else
					{
						playingSounds[i] = playingSounds[playingSoundsCount - 1];
						playingSoundsCount--;
						i--;
					}
				}
			}
		}
	}

	void SetupSound()
	{
		saudio_setup(
			{
				.stream_cb = AudioCallback,
				.logger =
				{
					.func = slog_func
				}
			}
		);
	}

	SoundClip loadSoundClip(const char* filePath)
	{
		// TODO: check if reading past the end of the file.

		uint8_t* file = FileSystem::loadEntireFile(filePath).ptr;

		// File .wav on wikipedia for all info.
		// Check if the RIFF constant is in the file -> constant which is always in .wav files.
		assert(file[0] == 'R');
		assert(file[1] == 'I');
		assert(file[2] == 'F');
		assert(file[3] == 'F');

		// goes 8 bytes faraway
		file += 4; // RIFF
		file += 4; // size

		assert(file[0] == 'W');
		assert(file[1] == 'A');
		assert(file[2] == 'V');
		assert(file[3] == 'E');

		file += 4; // WAVE

		// TODO: Look for the fmt block, which is not always following the first block
		assert(file[0] == 'f');
		assert(file[1] == 'm');
		assert(file[2] == 't');
		assert(file[3] == 0x20);

		file += 4; // fmt -> Information Block

		// BlockSize
		int32_t* ptr = (int32_t*)file;
		int32_t val = *ptr;
		printf("size: %i\n", val);

		file += 4; // size

		// AudioFormat (if 1 -> int)
		WaveFmtBlock* formatBlock = (WaveFmtBlock*)file;
		printf("audio format:   %i\n", formatBlock->wFormatTag);
		printf("nbr channels:   %i\n", formatBlock->nChannels);
		printf("sample per sec: %i\n", formatBlock->nSamplesPerSec);
		printf("bit per sample: %i\n", formatBlock->wBitsPerSample);

		file = (uint8_t*) ptr + 4 + 16;

		// for (int delta = 0; delta < 1000; delta++)
		// {
		//     file += 1;

		//     bool ok = file[0] == 'd' && file[1] == 'a' && file[2] == 't' && file[3] == 'a';

		//     if (ok)
		//     {
		//         printf("delta worked %i\n", delta);
		//         break;
		//     }
		// }

		file += 4; // data
		uint64_t dataSize = *(uint32_t*)file;
		file += 4; // data size

		int samplesCount = dataSize / sizeof(uint16_t) / 2;

		SoundClip clip =
			{
				clip.samples = (float*)malloc(sizeof(float) * samplesCount),
				clip.sampleCount = samplesCount,
				clip.samplePerSec = formatBlock->nSamplesPerSec
			};

		printf("samples: %i clipSamples; %i\n", samplesCount, clip.sampleCount);

		for (int i = 0; i < samplesCount; i++)
		{
			int16_t val = ((int)file[0] | file[1] << 8);
			file += 4; // 4 because we want to skip one of the channel

			clip.samples[i] = (float)val / (INT16_MAX); // 2^16 / 2 -> pour faire aller de -1 à 1
		}

		// // génére array de float avec des valeurs qui oscille entre 1 et -1 (sin)
		// float clipTime = 0;
		// for (int i = 0; i < clip.frameCount; i++)
		// {
		//     clip.samples[i] = sinf(clipTime * 2 * M_PI);
		//     clipTime += 1.0 / 44100 * 440;
		// }

		return clip;
	}

	Sound* AddSound(float amplitude, float frequency, float amplitudeFactor, float frequencyFactor, bool looping)
	{
		if (playingSoundsCount >= ARR_LEN(playingSounds))
		{
			printf("Too many sounds playing");
			return nullptr;
		}

		Sound sound =
		{
			0.0,
			amplitude, frequency,
			amplitudeFactor, frequencyFactor,
			looping,
		};

		playingSounds[playingSoundsCount++] = sound;

		return &playingSounds[playingSoundsCount - 1];
	}

	Sound* PlaySoundClip(SoundClip soundClip, float amplitude, float frequency, float amplitudeFactor, float frequencyFactor, bool looping)
	{
		Sound* sound = AddSound(amplitude, frequency, amplitudeFactor, frequencyFactor, looping);
		sound->clip = soundClip;

		return sound;
	}

	Sound* PlaySound(float amplitude, float frequency, float amplitudeFactor, float frequencyFactor, bool looping)
	{
		return AddSound(amplitude, frequency, amplitudeFactor, frequencyFactor, looping);
	}

	void StopSound(Sound& sound)
	{
		sound.amplitude = 0;
	}
}
