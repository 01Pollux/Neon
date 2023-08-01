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

    float GameTimer::GetTimeScale() const
    {
        return m_TimeScale;
    }

    void GameTimer::SetTimeScale(
        float TimeScale)
    {
        // We were paused and now we are unpaused
        if (m_TimeScale <= std::numeric_limits<float>::epsilon() && TimeScale > std::numeric_limits<float>::epsilon())
        {
            m_PrevTime = steady_clock::now();
        }
        m_TimeScale = TimeScale;
    }

    void GameTimer::Reset()
    {
        m_PrevTime  = steady_clock::now();
        m_BaseTime  = m_PrevTime;
        m_TimeScale = 1.f;

        m_GameTime = 0.0;
    }

    bool GameTimer::Tick()
    {
        if (m_TimeScale <= std::numeric_limits<float>::epsilon())
        {
            return false;
        }

        m_CurrTime     = steady_clock::now();
        auto DeltaTime = std::max(
            std::chrono::duration_cast<fmilliseconds>(m_CurrTime - m_PrevTime).count() * .001,
            0.0);
        m_DeltaTime = std::min(DeltaTime, 0.2) * m_TimeScale;
        m_PrevTime  = m_CurrTime;

        m_GameTime += m_DeltaTime;
        m_EngineTime += m_DeltaTime;

        return true;
    }
} // namespace Neon::Scene