#pragma once

#include "Time.h"

namespace BSE
{
    class DLL_EXPORT Engine
    {
    public:
        Engine() {}

        void DetectFrameAndTickRates(BSE::Time& time);

        static unsigned int GetCPUThreadCount();
        static uint64_t GetTotalRAM();
        static uint64_t GetAvailableRAM();
    };
}
