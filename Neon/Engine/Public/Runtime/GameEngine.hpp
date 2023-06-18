#pragma once

#include <Config/Engine.hpp>
#include <Runtime/Interface.hpp>

namespace Neon::Runtime
{
    class DefaultGameEngine : public InterfaceContainer
    {
    public:
        DefaultGameEngine() = default;

        DefaultGameEngine(const DefaultGameEngine&)            = delete;
        DefaultGameEngine& operator=(const DefaultGameEngine&) = delete;

        DefaultGameEngine(DefaultGameEngine&&)             = delete;
        DefaultGameEngine&& operator=(DefaultGameEngine&&) = delete;

        virtual ~DefaultGameEngine();

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

    protected:
        /// <summary>
        /// Load packs from config.
        /// </summary>
        void LoadPacks(
            const Config::EngineConfig& Config);

        /// <summary>
        /// Register splash screen pipeline
        /// </summary>
        void RegisterSplashScreenPipeline();
    };
} // namespace Neon::Runtime
