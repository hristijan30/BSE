#pragma once

#include "Time.h"

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#endif

namespace BSE
{
    class DLL_EXPORT Engine
    {
    public:
        Engine() = default;

        void DetectFrameAndTickRates(BSE::Time& time);

        static unsigned int GetCPUThreadCount();
        static uint64_t GetTotalRAM();
        static uint64_t GetAvailableRAM();
    };

#ifdef _WIN32
    inline bool CreateConsole()
    {
        if (AllocConsole())
        {
            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
            freopen_s(&fp, "CONIN$", "r", stdin);
            return true;
        }
        return false;
    }

    inline void DestroyConsole()
    {
        fclose(stdout);
        fclose(stderr);
        fclose(stdin);
        FreeConsole();
    }
#endif
}
