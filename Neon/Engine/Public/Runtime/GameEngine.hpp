#pragma once

#include <Config/Engine.hpp>

#include <Runtime/GameTimer.hpp>
#include <Runtime/Window.hpp>

#include <Asio/ThreadPool.hpp>

namespace Neon
{
    namespace Windowing
    {
        class IWindowApp;
    }

    namespace Scene
    {
        class GameScene;
    }
} // namespace Neon

namespace Neon::Runtime
{
    class DefaultGameEngine
    {
    public:
        DefaultGameEngine();

        NEON_CLASS_NO_COPY(DefaultGameEngine);
        NEON_CLASS_MOVE(DefaultGameEngine);

        virtual ~DefaultGameEngine();

        /// <summary>
        /// Get the singleton instance of the engine.
        /// </summary>
        static DefaultGameEngine* Get();

        /// <summary>
        /// Called when the engine is initialized.
        /// </summary>
        virtual void Initialize(
            Config::EngineConfig Config);

        /// <summary>
        /// Called when the engine is shutdown.
        /// </summary>
        virtual void Shutdown();

        /// <summary>
        /// Run the engine and return when the engine is closed or an error occurs.
        /// </summary>
        int Run();

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
        [[nodiscard]] Windowing::IWindowApp* GetWindow() const;

        /// <summary>
        /// Get the current scene.
        /// </summary>
        [[nodiscard]] Scene::GameScene& GetScene() const noexcept;

    public:
        /// <summary>
        /// Get the thread pool associated with the engine.
        /// </summary>
        auto GetThreadPool() const noexcept
        {
            return m_ThreadPool.get();
        }

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
        GameTimer                m_GameTimer;
        UPtr<EngineWindow>       m_Window;
        UPtr<Scene::GameScene>   m_Scene;
        UPtr<Asio::ThreadPool<>> m_ThreadPool;
    };
} // namespace Neon::Runtime
