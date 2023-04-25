#include <EnginePCH.hpp>
#include <Runtime/GameEngine.hpp>

#include <fstream>

namespace Neon
{
    DefaultGameEngine::DefaultGameEngine(
        const Config::EngineConfig& Config)
    {
        CreateWindow(Config.Window);
    }

    DefaultGameEngine::~DefaultGameEngine()
    {
    }

    int DefaultGameEngine::Run()
    {
        std::optional<int> ExitCode;
        Windowing::Event   Msg;

        while (!ExitCode)
        {
            while (m_Window->PeekEvent(&Msg))
            {
                if (auto Close = std::get_if<Windowing::Events::Close>(&Msg))
                {
                    ExitCode = Close->ExitCode;
                }
                else if (auto SizeChanged = std::get_if<Windowing::Events::SizeChanged>(&Msg))
                {
                    printf("%i,%i\n", SizeChanged->NewSize.Width(), SizeChanged->NewSize.Height());
                }
            }
        }

        return *ExitCode;
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
        m_Window.reset(Windowing::IWindowApp::Create(Config.Title, Config.Size, Style));

        std::vector<uint8_t> IconData(100 * 100 * 4 * 4);
        for (int i = 0; i < 400; i++)
        {
            for (int j = 0; j < 400; j += 4)
            {
                IconData[i * 100 + j + 0] = 0xFF;
                IconData[i * 100 + j + 1] = 0xFF;
                IconData[i * 100 + j + 2] = 0;
                IconData[i * 100 + j + 3] = 0xFF;
            }
        }

        m_Window->SetIcon(IconData.data(), { 100, 100 });
    }
} // namespace Neon
