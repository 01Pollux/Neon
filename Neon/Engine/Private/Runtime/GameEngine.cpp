#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>
#include <Module/Resource.hpp>
#include <Module/Graphics.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        LoadResourcePacks(Config.Resource);
        m_Window = std::make_unique<Module::Window>(this, Config);
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    int DefaultGameEngine::Run()
    {
        Initialize();
        auto Graphics = m_Window->GetGraphics();
        while (m_Window->Run())
        {
            m_ResourceManager->Run();
            Tick();

            Graphics->PreRender();
            Graphics->Render();
            Graphics->PostRender();
        }
        Shutdown();
        return m_Window->GetExitCode();
    }

    void DefaultGameEngine::LoadResourcePacks(
        const Config::ResourceConfig& Config)
    {
        m_ResourceManager = std::make_unique<Module::ResourceManager>(this, Config.Manager);
        for (auto& [Tag, Path] : Config.Packs)
        {
            m_ResourceManager->Get()->LoadPack(Tag, Path);
        }
    }
} // namespace Neon
