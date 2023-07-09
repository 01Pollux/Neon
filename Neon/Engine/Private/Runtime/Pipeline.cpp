#include <EnginePCH.hpp>
#include <Runtime/Pipeline.hpp>
#include <Runtime/PipelineBuilder.hpp>

#include <queue>
#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    EnginePipeline::EnginePipeline(
        EnginePipelineBuilder Builder,
        uint32_t              ThreadCount) :
        m_ThreadPool(ThreadCount)
    {
        std::queue<EnginePipelineBuilder::PipelinePhase*> CurrentLevel;

        auto Phases = &m_Levels.emplace_back();
        for (auto& [PhaseName, Phase] : Builder.m_Phases)
        {
            auto Iter = &m_Phases[PhaseName];
            if (!Phase.DependenciesCount)
            {
                if (!Phase.DependentNodes.empty())
                {
                    CurrentLevel.push(&Phase);
                }
                Phases->emplace_back(Iter);
            }
            for (auto& Child : Phase.DependentNodes)
            {
                auto ChildPhase = &m_Phases[Child->Name];
                ChildPhase->Parents.emplace_back(Iter);
            }
        }

        while (!CurrentLevel.empty())
        {
            Phases      = &m_Levels.emplace_back();
            size_t Size = CurrentLevel.size();
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
                        Phases->emplace_back(&m_Phases[Dependent->Name]);
                    }
                }
            }
        }
    }

    void EnginePipeline::BeginDispatch()
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
                        size_t ListenerCount;
                        {
                            std::scoped_lock Lock(Phase->Mutex);
                            ListenerCount = Phase->Signal.GetListenerCount();
                        }
                        if (!ListenerCount)
                        {
                            continue;
                        }

                        if (Phase->Flags.Test(EPipelineFlags::DontParallelize))
                        {
                            m_NonAsyncPhases.emplace_back(Phase);
                        }
                        else
                        {
                            Phase->Async = m_ThreadPool.Enqueue(
                                [this, Phase]
                                {
                                    for (auto Parent : Phase->Parents)
                                    {
                                        if (Parent->Async.valid())
                                            Parent->Async.wait();
                                    }
                                    std::scoped_lock Lock(Phase->Mutex);
                                    Phase->Signal.Broadcast();
                                });
                        }
                    }
                }
                for (auto Phase : m_NonAsyncPhases)
                {
                    std::scoped_lock Lock(Phase->Mutex);
                    Phase->Signal.Broadcast();
                }
                m_NonAsyncPhases.clear();
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

    void EnginePipeline::EndDispatch()
    {
        for (auto& Phases : m_Levels)
        {
            for (PipelinePhase* Phase : Phases)
            {
                if (Phase->Async.valid())
                    Phase->Async.wait();
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