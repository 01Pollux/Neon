#include <EnginePCH.hpp>
#include <Runtime/Pipeline.hpp>
#include <Runtime/PipelineBuilder.hpp>

#include <queue>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/async_scope.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/when_all_ready.hpp>

#include <Log/Logger.hpp>

namespace ranges = std::ranges;
namespace views  = std::views;

namespace Neon::Runtime
{
    EnginePipeline::EnginePipeline(
        EnginePipelineBuilder Builder,
        size_t                ThreadCount) :
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

    cppcoro::task<void> EnginePipeline::Dispatch()
    {
        for (auto& Passes : m_Levels)
        {
            std::vector<cppcoro::task<void>> Tasks;
            Tasks.reserve(Passes.size());

            for (size_t i = 0; i < Passes.size(); i++)
            {
                auto Phase = Passes[i];

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

                    if (Phase->Flags.Test(EPipelineFlags::DontParallelize) || ((i == Passes.size() - 1) && m_NonAsyncPhases.empty()))
                    {
                        m_NonAsyncPhases.emplace_back(Phase);
                    }
                    else
                    {
                        auto Task = [this](PipelinePhase* Phase) -> cppcoro::task<>
                        {
                            std::scoped_lock Lock(Phase->Mutex);
                            Phase->Signal.Broadcast();
                            co_return;
                        };
                        Tasks.emplace_back(cppcoro::schedule_on(m_ThreadPool, Task(Phase)));
                    }
                }
            }

            if (!Tasks.empty())
            {
                co_await cppcoro::when_all(std::move(Tasks));
            }

            for (auto Phase : m_NonAsyncPhases)
            {
                std::scoped_lock Lock(Phase->Mutex);
                Phase->Signal.Broadcast();
            }
            m_NonAsyncPhases.clear();
        }
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