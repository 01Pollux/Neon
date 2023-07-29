#pragma once

#include <chrono>

namespace Neon::Scene
{
    class GameTimer
    {
    public:
        GameTimer()
        {
            Start();
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
        /// Reset game timer
        /// </summary>
        void Reset();

        /// <summary>
        /// Start game timer
        /// </summary>
        void Start();

        /// <summary>
        /// Stop game timer
        /// </summary>
        void Stop();

        /// <summary>
        /// Update game timer
        /// </summary>
        void Tick();

    private:
        double m_DeltaTime  = 0;
        double m_GameTime   = 0;
        double m_EngineTime = 0;

        time_point m_BaseTime;
        time_point m_PrevTime;
        time_point m_CurrTime;

        bool m_Stopped = true;
    };
} // namespace Neon::Scene