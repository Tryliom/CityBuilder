#pragma once

namespace Random
{
    int Range(int min, int max);
    float Range(float min, float max);

    /**
     * Set a seed for the next random numbers
     * @param newSeed
     */
    void SetSeed(int newSeed);

    /**
     * Use a seed for the next random numbers
     * @param seed
     */
    void UseSeed();

    /**
     * Stop using a seed for the next random numbers
     */
    void StopUseSeed();

    int GetSeed();
}