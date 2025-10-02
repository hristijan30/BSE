#include "Engine.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

#include <thread>

namespace BSE
{
    void Engine::DetectFrameAndTickRates(BSE::Time& time)
    {
        static double accumulatedTime = 0.0;
        static int frameCount = 0;
        static int tickCount = 0;

        double instantaneousFPS = 1.0 / time.GetDeltaTime();

        if (time.ShouldTick())
        {
            tickCount++;
            time.ConsumeTick();
        }

        accumulatedTime += time.GetDeltaTime();
        frameCount++;

        double averageFPS = 0.0;
        double tickSpeed = 0.0;

        if (accumulatedTime >= 1.0)
        {
            averageFPS = frameCount / accumulatedTime;
            tickSpeed = tickCount / accumulatedTime;

            std::cout << "Instantaneous FPS: " << instantaneousFPS << "\n";
            std::cout << "Average FPS: " << averageFPS << "\n";
            std::cout << "Tick Speed (TPS): " << tickSpeed << "\n\n";

            accumulatedTime = 0.0;
            frameCount = 0;
            tickCount = 0;
        }
    }

    unsigned int Engine::GetCPUThreadCount()
    {
        unsigned int threads = std::thread::hardware_concurrency();
        return threads > 0 ? threads : 1;
    }

    uint64_t Engine::GetTotalRAM()
    {
#if defined(_WIN32)
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        return memInfo.ullTotalPhys;
#elif defined(__linux__)
        struct sysinfo memInfo;
        sysinfo(&memInfo);
        return memInfo.totalram * memInfo.mem_unit;
#endif
    }

    uint64_t Engine::GetAvailableRAM()
    {
#if defined(_WIN32)
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        return memInfo.ullAvailPhys;
#elif defined(__linux__)
        struct sysinfo memInfo;
        sysinfo(&memInfo);
        return memInfo.freeram * memInfo.mem_unit;
#endif
    }
}