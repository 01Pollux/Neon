#pragma once

#include <Config/Engine.hpp>

#include <Runtime/Interface.hpp>
#include <Runtime/Window.hpp>

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
    class DefaultGameEngine : public InterfaceContainer
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
        virtual void Shutdown()
        {
        }

        /// <summary>
        /// Run the engine and return when the engine is closed or an error occurs.
        /// </summary>
        int Run();

    public:
        /// <summary>
        /// Get the window associated with the engine.
        /// </summary>
        [[nodiscard]] Windowing::IWindowApp* GetWindow() const;

        /// <summary>
        /// Get the current scene.
        /// </summary>
        [[nodiscard]] Scene::GameScene& GetScene() const noexcept;

        /// <summary>
        /// Set the current scene.
        /// </summary>
        void SetScene(
            UPtr<Scene::GameScene> Scene);

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
        UPtr<EngineWindow>     m_Window;
        UPtr<Scene::GameScene> m_Scene;
    };
} // namespace Neon::Runtime
