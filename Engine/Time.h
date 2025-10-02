#pragma once

#include "Define.h"
#include "StandardInclude.h"

namespace BSE
{
    class DLL_EXPORT Time
    {
    public:
        Time(double tickRate = 60.0)
            : m_tickRate(tickRate),
              m_timePerTick(1.0 / tickRate),
              m_accumulator(0.0),
              m_previousTime(std::chrono::high_resolution_clock::now()) {}

        void Update();
        bool ShouldTick() const;
        void ConsumeTick();

        double GetDeltaTime() const { return m_frameTime; }

        double GetTimePerTick() const { return m_timePerTick; }

        double GetAlpha() const { return m_accumulator / m_timePerTick; } // Use for graphical updates
    
    private:
        double m_tickRate;
        double m_timePerTick;
        double m_accumulator;
        double m_frameTime;

        std::chrono::high_resolution_clock::time_point m_previousTime;
    };
}