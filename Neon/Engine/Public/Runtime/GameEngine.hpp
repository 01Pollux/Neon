#pragma once

#include <Config/Engine.hpp>
#include <Runtime/GameTimer.hpp>
#include <Asio/ThreadPool.hpp>

namespace Neon
{
    namespace Windowing
    {
        class WindowApp;
    }
} // namespace Neon

struct GLFWwindow;

namespace Neon::Runtime
{
    class GameLogic;

    class GameEngine
    {
    public:
        GameEngine();

        NEON_CLASS_NO_COPY(GameEngine);
        NEON_CLASS_MOVE(GameEngine);

        virtual ~GameEngine();

        /// <summary>
        /// Get the singleton instance of the engine.
        /// </summary>
        static GameEngine* Get();

        /// <summary>
        /// Called when the engine is initialized.
        /// </summary>
        void Initialize(
            Config::EngineConfig Config);

        /// <summary>
        /// Called when the engine is shutdown.
        /// </summary>
        virtual void Shutdown()
        {
        }

        /// <summary>
        /// Called before the engine update.
        /// </summary>
        virtual void PreUpdate()
        {
        }

        /// <summary>
        /// Called after the engine update.
        /// </summary>
        virtual void PostUpdate()
        {
        }

        /// <summary>
        /// Called before the engine render.
        /// </summary>
        virtual void PreRender()
        {
        }

        /// <summary>
        /// Called after the engine render.
        /// </summary>
        virtual void PostRender()
        {
        }

        /// <summary>
        /// Run the engine and return when the engine is closed or an error occurs.
        /// </summary>
        void Run();

    public:
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

    public:
        /// <summary>
        /// Get the window associated with the engine.
        /// </summary>
        [[nodiscard]] Windowing::WindowApp* GetWindow() const;

        /// <summary>
        /// Get the window handle associated with the engine.
        /// </summary>
        [[nodiscard]] GLFWwindow* GetWindowHandle() const;

        /// <summary>
        /// Get the current scene.
        /// </summary>
        [[nodiscard]] GameLogic* GetLogic() const noexcept;

        /// <summary>
        /// Get the global thread pool.
        /// </summary>
        Asio::ThreadPool<>& GetThreadPool() noexcept;

    protected:
        /// <summary>
        /// Load packs from config.
        /// </summary>
        void LoadPacks(
            Config::EngineConfig& Config);

        /// <summary>
        /// Initialize assets system.
        /// </summary>
        void InitializeAssetSystem(
            Config::EngineConfig& Config);

    private:
        GameTimer                  m_GameTimer;
        UPtr<Windowing::WindowApp> m_Window;
        UPtr<GameLogic>            m_Logic;
        Asio::ThreadPool<>         m_ThreadPool{ 3 };
    };
} // namespace Neon::Runtime
