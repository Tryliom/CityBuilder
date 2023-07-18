#include "Random.h"

#include <random>

int seed = 0;
int offset = 0;
bool useSeed = false;

namespace Random
{
    int Range(int min, int max)
    {
        std::mt19937 rng(useSeed ? seed + offset : std::random_device()());
        std::uniform_int_distribution<int> dist(min, max);

        if (useSeed)
        {
	        offset++;
        }

        return dist(rng);
    }

    float Range(float min, float max)
    {
        std::mt19937 rng(useSeed ? seed + offset : std::random_device()());
        std::uniform_real_distribution<float> dist(min, max);

        if (useSeed)
        {
	        offset++;
        }

        return dist(rng);
    }

    void SetSeed(int newSeed)
    {
        seed = newSeed;
    }

    void UseSeed()
    {
        useSeed = true;
	    offset = 0;
    }

    void StopUseSeed()
    {
        useSeed = false;
    }

    int GetSeed()
    {
        return seed;
    }
}