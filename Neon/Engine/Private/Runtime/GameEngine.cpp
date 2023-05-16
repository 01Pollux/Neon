#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Window.hpp>
#include <Module/Resource.hpp>

//
#include <RHI/Device.hpp>
#include <RHI/Swapchain.hpp>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        LoadResourcePacks(Config.Resource);
        CreateWindow(Config.Window);

        RHI::IRenderDevice::CreateGlobal();

        RHI::ISwapchain::InitDesc Desc{
            .Window = m_World.GetModule<Module::Window>()->GetWindow(),
        };
        m_Swapchain.reset(RHI::ISwapchain::Create(Desc));
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
        m_Swapchain.reset();
        RHI::IRenderDevice::DestroyGlobal();
    }

    int DefaultGameEngine::Run()
    {
        while (!m_World->should_quit())
        {
            m_World->progress();
            m_Swapchain->PrepareFrame();
            m_Swapchain->Present();
        }
        return m_World.GetModule<Module::Window>()->GetExitCode();
    }

    World& DefaultGameEngine::GetWorld()
    {
        return m_World;
    }

    //

    void DefaultGameEngine::CreateWindow(
        const Config::WindowConfig& Config)
    {
        Windowing::MWindowStyle Style;
        if (Config.WithCloseButton)
        {
            Style.Set(Windowing::EWindowStyle::Close);
        }
        if (Config.CanResize)
        {
            Style.Set(Windowing::EWindowStyle::Resize);
        }
        if (Config.HasTitleBar)
        {
            Style.Set(Windowing::EWindowStyle::TitleBar);
        }
        if (Config.Windowed)
        {
            Style.Set(Windowing::EWindowStyle::Windowed);
        }
        if (Config.Fullscreen)
        {
            Style.Set(Windowing::EWindowStyle::Fullscreen);
        }
        m_World.Import<Module::Window>(Config.Title, Config.Size, Style, Config.StartInMiddle);
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
