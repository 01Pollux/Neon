#pragma once

#include <chrono>

namespace Neon::Scene
{
    class GameTimer
    {
    public:
        GameTimer()
        {
            SetTimeScale(1.f);
        }

        using steady_clock = std::chrono::steady_clock;
        using time_point   = steady_clock::time_point;
        using seconds_type = std::chrono::seconds;

        /// <summary>
        /// Get total elapsed time for the current level
        /// </summary>
        [[nodiscard]] double GetGameTime() const;

        /// <summary>
        /// Get total elapsed time for the current engine
        /// </summary>
        [[nodiscard]] double GetEngineTime() const;

        /// <summary>
        /// Get delta time
        /// </summary>
        [[nodiscard]] double GetDeltaTime() const;

        /// <summary>
        /// Get time scale
        /// </summary>
        [[nodiscard]] float GetTimeScale() const;

        /// <summary>
        /// Set time scale
        /// </summary>
        void SetTimeScale(
            float TimeScale);

        /// <summary>
        /// Reset game timer
        /// </summary>
        void Reset();

        /// <summary>
        /// Update game timer, returns true if game is running else false
        /// </summary>
        [[nodiscard]] bool Tick();

    private:
        double m_DeltaTime  = 0;
        double m_GameTime   = 0;
        double m_EngineTime = 0;
        float  m_TimeScale  = 1.0f;

        time_point m_BaseTime;
        time_point m_PrevTime;
        time_point m_CurrTime;
    };
} // namespace Neon::Scene