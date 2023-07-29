#include <EnginePCH.hpp>
#include <Scene/GameTimer.hpp>

namespace Neon::Scene
{
    using fmilliseconds = std::chrono::duration<double, std::milli>;

    double GameTimer::GetGameTime() const
    {
        return m_GameTime;
    }

    double GameTimer::GetEngineTime() const
    {
        return m_EngineTime;
    }

    double GameTimer::GetDeltaTime() const
    {
        return m_DeltaTime;
    }

    void GameTimer::Reset()
    {
        m_PrevTime = steady_clock::now();
        m_BaseTime = m_PrevTime;
        m_Stopped  = false;

        m_GameTime = 0.0;
    }

    void GameTimer::Start()
    {
        if (m_Stopped)
        {
            m_PrevTime = steady_clock::now();
            m_Stopped  = false;
        }
    }

    void GameTimer::Stop()
    {
        m_Stopped = true;
    }

    void GameTimer::Tick()
    {
        if (m_Stopped)
        {
            m_DeltaTime = 0.0;
            return;
        }

        m_CurrTime  = steady_clock::now();
        m_DeltaTime = std::max(
            std::chrono::duration_cast<fmilliseconds>(m_CurrTime - m_PrevTime).count() * .001,
            0.0);
        m_PrevTime = m_CurrTime;

        m_GameTime += m_DeltaTime;
        m_EngineTime += m_DeltaTime;
    }
} // namespace Neon::Scene