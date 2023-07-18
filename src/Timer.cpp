#include "Timer.h"

#include "sokol_app.h"
#include "Logger.h"

int frames = 0;
float timeStack[200];

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
                    timeStack[i] = DeltaTime;
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

        DeltaTime = sapp_frame_duration();
        timeStack[frames - 1] = DeltaTime;
        Time += DeltaTime;

        auto sum = 0.f;

        for (auto i : timeStack)
        {
            sum += i;
        }

        SmoothDeltaTime = sum / frames;
    }
}