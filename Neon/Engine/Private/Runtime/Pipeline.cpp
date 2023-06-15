#include <EnginePCH.hpp>
#include <Runtime/Pipeline.hpp>
#include <Runtime/PipelineBuilder.hpp>

#include <queue>
#include <execution>
#include <iostream>

#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    EnginePipeline::EnginePipeline(
        EnginePipelineBuilder Builder)
    {
        std::queue<EnginePipelineBuilder::PipelinePhase*> CurrentLevel;

        auto& Phases = m_Levels.emplace_back();
        for (auto& [PhaseName, Phase] : Builder.m_Phases)
        {
            NEON_ASSERT(!m_Phases.contains(PhaseName));
            auto Iter = &m_Phases[PhaseName];
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

    void EnginePipeline::Dispatch()
    {
        if (m_ThreadPool.GetThreadsCount())
        {
            for (auto& Phases : m_Levels)
            {
                for (size_t i = 0; i < Phases.size(); i++)
                {
                    auto Phase = Phases[i];
                    if (!Phase->Flags.Test(EPipelineFlags::Disabled))
                    {
                        if (Phase->Flags.Test(EPipelineFlags::DontParallelize))
                        {
                            m_NonAsyncPhases.emplace_back(Phase);
                        }
                        else
                        {
                            // Run the last task on the main thread
                            if (i == Phases.size() && m_NonAsyncPhases.empty())
                            {
                                std::scoped_lock Lock(Phase->Mutex);
                                Phase->Signal.Broadcast();
                            }
                            else
                            {
                                m_AsyncPhases.emplace_back(m_ThreadPool.Enqueue(
                                    [this, Phase]
                                    {
                                        std::scoped_lock Lock(Phase->Mutex);
                                        Phase->Signal.Broadcast();
                                    }));
                            }
                        }
                    }
                }
                for (auto Phase : m_NonAsyncPhases)
                {
                    std::scoped_lock Lock(Phase->Mutex);
                    Phase->Signal.Broadcast();
                }
                m_NonAsyncPhases.clear();

                std::for_each(
                    std::execution::par_unseq,
                    m_AsyncPhases.begin(),
                    m_AsyncPhases.end(),
                    [](auto& Future)
                    { Future.wait(); });
                m_AsyncPhases.clear();
            }
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

    void EnginePipeline::SetThreadCount(
        size_t ThreadCount)
    {
        m_ThreadPool.Resize(ThreadCount);
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