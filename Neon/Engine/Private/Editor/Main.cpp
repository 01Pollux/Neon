#include <EnginePCH.hpp>
#include <Runtime/EntryPoint.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <iostream>
#include <boost/program_options.hpp>

//

namespace bpo = boost::program_options;

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = { .Title = "Neon Editor" },
        .Renderer{ .Device = { .EnableDebugLayer = false, .EnableGPUDebugger = true/*, .EnableGpuBasedValidation = true*/ } }
    };

    {
        bpo::options_description Description("Allowed options");
        Description.add_options()(
            "help", "Produce help message")(

            "custom_titlebar,ctb", bpo::value<bool>()->default_value(true)->notifier([&](bool Val)
                                                                                     { Config.Window.CustomTitleBar = Val; }),

            "Run maximized")(
            "maximized,m", bpo::value<bool>()->default_value(false)->notifier([&](bool Val)
                                                                              { Config.Window.Maximized = Val; }),

            "Run maximized")(
            "fullscreen", bpo::value<bool>()->default_value(false)->notifier([&](bool Val)
                                                                             { Config.Window.FullScreen = Val; }),

            "Run in fullscreen mode")(

            "width,w", bpo::value<int>()->default_value(1280)->notifier([&](int Val)
                                                                        { Config.Window.Size.x = Val; }),
            "Set window width")(

            "height,h", bpo::value<int>()->default_value(720)->notifier([&](int Val)
                                                                        { Config.Window.Size.y = Val; }),
            "Set window height")(

            "start-in-middle,sim", bpo::value<bool>()->default_value(true)->notifier([&](bool Val)
                                                                                     { Config.Window.StartInMiddle = Val; }),
            "Start window in the middle of the screen");

        bpo::variables_map Vars;
        bpo::store(bpo::parse_command_line(Argc, Argv, Description), Vars);
        bpo::notify(Vars);

        if (Vars.count("help"))
        {
            std::cout << Description << std::endl;
            return 0;
        }
    }

    return RunEngine<Editor::EditorEngine>(std::move(Config));
}
