#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>

#include <Renderer/Renderer.hpp>

namespace Neon::Runtime
{
    GameLogic::GameLogic(
        DefaultGameEngine* Engine) :
        m_Engine(Engine)
    {
        // First attempt

        // PipelineBuilder Builder;
        //
        // auto Renderer = Builder.NewPhase("Renderer");
        // auto Audio = Builder.NewPhase("Audio");
        // auto Physics = Builder.NewPhase("Physics");
        // auto Input = Builder.NewPhase("Input");
        // auto Logic = Builder.NewPhase("Logic");
        // auto Launcher = Builder.NewPhase("Launcher");
        //
        // Audio.DependsOn(Logic);
        // Physics.DependsOn(Logic);
        // Renderer.DependsOn(Logic);
        // Input.DependsOn(Logic);
        // Logic.DependsOn(Launcher);
        //
        // EnginePipeline Pipeline(Builder);
        //
        // Pipeline.Dispatch();
        //
        // Pipeline.GetPhases();
        // Pipeline.GetPhase("Renderer").Attach(
        //  []() { std::cout << "Renderer" << std::endl; });
    }

    GameLogic::~GameLogic() = default;

    void GameLogic::Tick()
    {
    }

    void GameLogic::Render()
    {
        if (m_Renderer)
        {
            m_Renderer->Render();
        }
    }
} // namespace Neon::Runtime
