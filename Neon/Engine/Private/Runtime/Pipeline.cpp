#include <EnginePCH.hpp>
#include <Runtime/Pipeline.hpp>
#include <Runtime/PipelineBuilder.hpp>
#include <queue>

namespace Neon::Runtime
{
    EnginePipeline::EnginePipeline(
        EnginePipelineBuilder Builder) :
        m_ThreadCount(std::thread::hardware_concurrency() / 2)
    {
        std::queue<EnginePipelineBuilder::PipelinePhase*> CurrentLevel;

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

    void EnginePipeline::BeginPhases()
    {
        auto RunLevels = [this]()
        {
            auto ThreadCount = m_ThreadCount;
            for (auto& Phases : m_Levels)
            {
                std::vector<std::jthread> WaitingPhases;
                for (PipelinePhase* Phase : Phases)
                {
                    if (!Phase->Flags.Test(EPipelineFlags::Disabled))
                    {
                        bool Parallelize = !Phase->Flags.Test(EPipelineFlags::DontParallelize);

                        std::unique_lock Lock(m_ExecuteMutex, std::defer_lock);
                        if (Parallelize)
                        {
                            Lock.lock();
                            Parallelize = ThreadCount > 0;
                        }

                        if (Parallelize)
                        {
                            Lock.unlock();
                            Phase->Signal.Broadcast();
                        }
                        else
                        {
                            m_ThreadCount--;
                            Lock.unlock();
                            WaitingPhases.emplace_back(
                                [Phase, this]
                                {
                                    Phase->Signal.Broadcast();
                                    std::unique_lock Lock(m_ExecuteMutex);
                                    m_ThreadCount++;
                                });
                        }
                    }
                }
            }
        };

        if (m_ThreadCount)
        {
            m_ExecutionThread = std::jthread(RunLevels);
        }
        else
        {
            for (auto& Phases : m_Levels)
            {
                for (PipelinePhase* Phase : Phases)
                {
                    Phase->Signal.Broadcast();
                }
            }
        }
    }

    void EnginePipeline::EndPhases()
    {
        if (m_ExecutionThread.joinable())
            m_ExecutionThread.join();
    }

    void EnginePipeline::SetThreadCount(
        uint32_t ThreadCount)
    {
        m_ThreadCount = ThreadCount;
    }

    void EnginePipeline::SetPhaseEnable(
        const StringU8& PhaseName,
        bool            Enabled)
    {
        auto& Phase = m_Phases.find(PhaseName)->second;
        Phase.Flags.Set(EPipelineFlags::Disabled, !Enabled);
    }

    void EnginePipeline::SetPhaseParallelize(
        const StringU8& PhaseName,
        bool            Parallelize)
    {
        auto& Phase = m_Phases.find(PhaseName)->second;
        Phase.Flags.Set(EPipelineFlags::DontParallelize, !Parallelize);
    }
} // namespace Neon::Runtime