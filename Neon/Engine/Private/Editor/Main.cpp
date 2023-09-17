#include <EnginePCH.hpp>
#include <Runtime/EntryPoint.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <iostream>
#include <boost/program_options.hpp>

//

namespace bpo = boost::program_options;

NEON_MAIN(Argc, Argv)
{
    Config::EditorConfig Config{
        { .Window = { .Title = "Neon Editor" },
          .Renderer{ .Device = { .EnableDebugLayer = false, .EnableGPUDebugger = true /*, .EnableGpuBasedValidation = true*/ } } }
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
            "Start window in the middle of the screen")(

            "project-path,p", bpo::value<StringU8>()->notifier([&](const StringU8& Val)
                                                               { Config.StartupProjectPath = Val; }),
            "Project path (containing .neon file) to load")(

            "new-project-name,n", bpo::value<StringU8>()->notifier([&](const StringU8& Val)
                                                                   { Config.NewProjectName = Val; }),
            "Creates a new project with name");

        bpo::variables_map Vars;

        try
        {
            bpo::store(bpo::parse_command_line(Argc, Argv, Description), Vars);
        }
        catch (const bpo::error& Error)
        {
            NEON_FATAL(Error.what());
            return nullptr;
        }

        bpo::notify(Vars);

        if (Vars.count("help"))
        {
            std::cout << Description << std::endl;
            return nullptr;
        }

        NEON_ASSERT(Config.Window.Size.x > 0 && Config.Window.Size.y > 0, "Invalid window size");
        NEON_ASSERT(!Config.StartupProjectPath.empty(), "No project path or name specified");
    }

    return RunEngine<Editor::EditorEngine>(std::move(Config));
}
