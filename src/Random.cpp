#include "Random.h"

#include <random>

int seed = 0;
bool useSeed = false;

namespace Random
{
    int Range(int min, int max)
    {
        std::mt19937 rng(useSeed ? seed : std::random_device()());
        std::uniform_int_distribution<int> dist(min, max);

        if (useSeed)
        {
            seed++;
        }

        return dist(rng);
    }

    float Range(float min, float max)
    {
        std::mt19937 rng(useSeed ? seed : std::random_device()());
        std::uniform_real_distribution<float> dist(min, max);

        if (useSeed)
        {
            seed++;
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
    }

    void StopUseSeed()
    {
        useSeed = false;
    }
}