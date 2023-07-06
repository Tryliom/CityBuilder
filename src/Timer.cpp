#include "Timer.h"

#include "sokol_app.h"

float deltaTime = 0.f;
int frames = 0;
float timeStack[200];
float time;

namespace Timer
{
    void Update()
    {
        if (frames == 200)
        {
            for (int i = 0; i < 200; i++)
            {
                if (i == 199)
                {
                    timeStack[i] = deltaTime;
                }
                else
                {
                    timeStack[i] = timeStack[i + 1];
                }
            }
        }

        if (frames < 200)
        {
            frames++;
        }

        deltaTime = sapp_frame_duration();
        timeStack[frames - 1] = deltaTime;
        time += deltaTime;
    }

    float GetSmoothDeltaTime()
    {
        auto sum = 0.f;

        for (auto i : timeStack)
        {
            sum += i;
        }

        return sum / frames;
    }

    float GetDeltaTime()
    {
        return deltaTime;
    }

    float GetTime()
    {
        return time;
    }
}