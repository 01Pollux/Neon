#pragma once

#include <Config/Engine.hpp>
#include <Utils/Singleton.hpp>

#include <Runtime/Interface.hpp>
#include <Runtime/Window.hpp>
#include <Runtime/Pipeline.hpp>

#include <optional>

namespace Neon
{
    namespace Windowing
    {
        class IWindowApp;
    }
} // namespace Neon

namespace Neon::Runtime
{
    class DefaultGameEngine : public InterfaceContainer,
                              public Utils::Singleton<DefaultGameEngine>
    {
    public:
        DefaultGameEngine() = default;

        NEON_CLASS_NO_COPY(DefaultGameEngine);
        NEON_CLASS_MOVE(DefaultGameEngine);

        virtual ~DefaultGameEngine() = default;

        /// <summary>
        /// Called when the engine is initialized.
        /// </summary>
        virtual void Initialize(
            const Config::EngineConfig& Config);

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
        /// Get the pipeline associated with the engine.
        /// </summary>
        [[nodiscard]] EnginePipeline* GetPipeline() const;

        /// <summary>
        /// Set the pipeline associated with the engine.
        /// </summary>
        void SetPipeline(
            UPtr<EnginePipeline> Pipeline);

    protected:
        /// <summary>
        /// Load packs from config.
        /// </summary>
        void LoadPacks(
            const Config::EngineConfig& Config);

    private:
        UPtr<EngineWindow>   m_Window;
        UPtr<EnginePipeline> m_Pipeline;

        std::optional<UPtr<EnginePipeline>> m_PendingPipeline;
    };
} // namespace Neon::Runtime
