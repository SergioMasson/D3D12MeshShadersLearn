#include "Engine/SystemTime.h"
#include "Engine/Utility.h"

#include <windows.h>

namespace Engine 
{
    double Engine::SystemTime::sm_CpuTickDelta = 0.0;

    int64_t g_lastFrame = 0;

    // Query the performance counter frequency
    void SystemTime::Initialize(void)
    {
        LARGE_INTEGER frequency;
        ENGINE_ASSERT(TRUE == QueryPerformanceFrequency(&frequency), "Unable to query performance counter frequency");
        sm_CpuTickDelta = 1.0 / static_cast<double>(frequency.QuadPart);
        g_lastFrame = GetCurrentTick();
    }

    // Query the current value of the performance counter
    int64_t SystemTime::GetCurrentTick(void)
    {
        LARGE_INTEGER currentTick;
        ENGINE_ASSERT(TRUE == QueryPerformanceCounter(&currentTick), "Unable to query performance counter value");
        return static_cast<int64_t>(currentTick.QuadPart);
    }

    double SystemTime::GetCurrentFrameTime(void)
    {
        auto currentFrame = GetCurrentTick();
        auto frameTime = TicksToMillisecs(currentFrame - g_lastFrame);
        g_lastFrame = currentFrame;
        return frameTime;
    }

    void SystemTime::BusyLoopSleep(float SleepTime)
    {
        int64_t finalTick = (int64_t)((double)SleepTime / sm_CpuTickDelta) + GetCurrentTick();
        while (GetCurrentTick() < finalTick);
    }
}