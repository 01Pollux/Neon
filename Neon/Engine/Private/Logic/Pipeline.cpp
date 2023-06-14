#include <EnginePCH.hpp>
#include <Logic/Pipeline.hpp>
#include <Logic/PipelineBuilder.hpp>
#include <queue>

namespace Neon
{
    Pipeline::Pipeline(
        PipelineBuilder Builder) :
        m_ThreadCount(std::thread::hardware_concurrency() / 2)
    {
        std::queue<PipelineBuilder::PipelinePhase*> CurrentLevel;

        auto& Phases = m_Levels.emplace_back();
        for (auto& [PhaseName, Phase] : Builder.m_Phases)
        {
            auto Iter = &m_Phases.emplace(PhaseName, PipelinePhase{}).first->second;
            if (!Phase.DependenciesCount)
            {
                if (!Phase.DependentNodes.empty())
                {
                    CurrentLevel.push(&Phase);
                }
                Phases.emplace_back(Iter);
            }
        }

        while (!CurrentLevel.empty())
        {
            auto&  Phases = m_Levels.emplace_back();
            size_t Size   = CurrentLevel.size();
            for (size_t i = 0; i < Size; i++)
            {
                auto CurrentPhase = CurrentLevel.front();
                CurrentLevel.pop();

                for (auto& Dependent : CurrentPhase->DependentNodes)
                {
                    Dependent->DependenciesCount--;
                    if (!Dependent->DependenciesCount)
                    {
                        if (!Dependent->DependentNodes.empty())
                        {
                            CurrentLevel.push(Dependent);
                        }
                        Phases.emplace_back(&m_Phases[Dependent->Name]);
                    }
                }
            }
        }
    }

    void Pipeline::BeginPhases()
    {
        m_ExecutionThread = std::jthread(
            [this]()
            {
                for (auto& Phases : m_Levels)
                {
                    std::vector<std::jthread> WaitingPhases;
                    for (PipelinePhase* Phase : Phases)
                    {
                        if (Phase->Flags.Test(EPipelineFlags::DontParallelize))
                            Phase->Signal.Broadcast();
                        else
                        {
                            WaitingPhases.emplace_back(
                                [Phase]()
                                { Phase->Signal.Broadcast(); });
                        }
                    }
                }
            });
    }

    void Pipeline::EndPhases()
    {
        m_ExecutionThread.join();
    }

    void Pipeline::SetThreadCount(
        uint32_t ThreadCount)
    {
        m_ThreadCount = ThreadCount;
    }

    void Pipeline::SetPhaseEnable(
        const StringU8& PhaseName,
        bool            Enabled)
    {
        auto& Phase = m_Phases.find(PhaseName)->second;
        Phase.Flags.Set(EPipelineFlags::Disabled, !Enabled);
    }

    void Pipeline::SetPhaseParallelize(
        const StringU8& PhaseName,
        bool            Parallelize)
    {
        auto& Phase = m_Phases.find(PhaseName)->second;
        Phase.Flags.Set(EPipelineFlags::DontParallelize, !Parallelize);
    }
} // namespace Neon