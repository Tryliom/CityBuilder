#pragma once

struct TimerData
{
    float Time = 0.f;
    float DeltaTime = 0.f;
    float SmoothDeltaTime = 0.f; 
};

namespace Timer
{
    inline float Time = 0.f;
    inline float DeltaTime = 0.f;
    inline float SmoothDeltaTime = 0.f; 

    void Update();
}