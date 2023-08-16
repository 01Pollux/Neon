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
        .Window = { .Title = STR("Neon Editor") }
    };

    {
        bpo::options_description Description("Allowed options");
        Description.add_options()(
            "help", "Produce help message")(

            "windowed", bpo::value<bool>()->default_value(true)->notifier([&](bool Val)
                                                                          { Config.Window.Windowed = Val; }),

            "Run in windowed mode")(
            "fullscreen", bpo::value<bool>()->default_value(false)->notifier([&](bool Val)
                                                                             { Config.Window.Fullscreen = Val; }),

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
