#pragma once

namespace Timer
{
    void Init();
    void Update();

    float GetSmoothDeltaTime();
    float GetDeltaTime();
    float GetTime();
}