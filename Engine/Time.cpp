#include "Time.h"

namespace BSE
{
    void Time::Update()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> delta = currentTime - m_previousTime;
        m_previousTime = currentTime;

        m_frameTime = delta.count();
        m_accumulator += m_frameTime;

        if (m_accumulator > 0.25) m_accumulator = 0.25;
    }

    bool Time::ShouldTick() const {
        return m_accumulator >= m_timePerTick;
    }

    void Time::ConsumeTick()
    {
        m_accumulator -= m_timePerTick;
    }
}