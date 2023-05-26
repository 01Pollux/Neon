#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>
#include <Module/Resource.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        if (Config.EnableFlecsREST)
        {
            m_World->set<flecs::Rest>({});
            m_World->import <flecs::monitor>();
        }

        LoadResourcePacks(Config.Resource);
        m_World.Import<Module::Window>(Config);
    }

    int DefaultGameEngine::Run()
    {
        while (!m_World->should_quit())
        {
            m_World->progress();
        }
        return m_World.GetModule<Module::Window>()->GetExitCode();
    }

    World& DefaultGameEngine::GetWorld()
    {
        return m_World;
    }

    void DefaultGameEngine::LoadResourcePacks(
        const Config::ResourceConfig& Config)
    {
        auto Manager = m_World.Import<Module::ResourceManager>(Config.Manager)->Get();
        for (auto& [Tag, Path] : Config.Packs)
        {
            Manager->LoadPack(Tag, Path);
        }
    }
} // namespace Neon
