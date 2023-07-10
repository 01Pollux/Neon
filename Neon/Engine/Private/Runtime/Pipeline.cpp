#include <EnginePCH.hpp>
#include <Runtime/Pipeline.hpp>
#include <Runtime/PipelineBuilder.hpp>

#include <queue>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/when_all_ready.hpp>

#include <Log/Logger.hpp>

namespace ranges = std::ranges;
namespace views  = std::views;

namespace Neon::Runtime
{
    EnginePipeline::EnginePipeline(
        EnginePipelineBuilder Builder) :
        m_ThreadPool(4)
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

        auto FindInstance = [this](PipelinePhase* Phase) -> StringU8
        {
            for (auto& [Name, Instance] : m_Phases)
            {
                if (&Instance == Phase)
                {
                    return Name;
                }
            }
            return "";
        };
    }

    void EnginePipeline::BeginDispatch()
    {
        for (size_t i = 0; i < m_Levels.size(); i++)
        {
            auto& Phase = m_Levels[i];
            for (size_t j = 0; j < Phase.size(); j++)
            {
                auto CurPhase = Phase[j];
                if (!CurPhase->Flags.Test(EPipelineFlags::Disabled))
                {
                    size_t ListenerCount;
                    {
                        std::scoped_lock Lock(CurPhase->Mutex);
                        ListenerCount = CurPhase->Signal.GetListenerCount();
                    }
                    if (!ListenerCount)
                    {
                        continue;
                    }

                    CurPhase->Flags.Set(EPipelineFlags::Executing, true);
                    if (CurPhase->Flags.Test(EPipelineFlags::DontParallelize) && false)
                    {
                        m_NonAsyncPhases.emplace_back(CurPhase);
                    }
                    else
                    {
                        auto Task = [](EnginePipeline* Pipeline, size_t i, size_t j) -> cppcoro::shared_task<>
                        {
                            auto                                CurPhase = Pipeline->m_Levels[i][j];
                            std::vector<cppcoro::shared_task<>> ParentTasks;
                            ParentTasks.reserve(CurPhase->Parents.size());
                            for (auto Parent : CurPhase->Parents)
                            {
                                ParentTasks.emplace_back(Parent->Task);
                            }

                            // auto ParentTasks = CurPhase->Parents |
                            //                    views::transform(
                            //                        [](PipelinePhase* Parent)
                            //                        {
                            //                            return Parent->Task;
                            //                        }) |
                            //                    ranges::to<std::vector>();

                            cppcoro::sync_wait(cppcoro::when_all_ready(ParentTasks));

                            std::scoped_lock Lock(CurPhase->Mutex);
                            CurPhase->Signal.Broadcast();
                            co_return;
                        };

                        CurPhase->Task = cppcoro::make_shared_task(cppcoro::schedule_on(m_ThreadPool, Task(this, i, j)));
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

    void EnginePipeline::EndDispatch()
    {
        std::vector<cppcoro::shared_task<>> Tasks;

        for (auto& Phase : m_Levels)
        {
            auto PhaseTasks = Phase |
                              views::take_while(
                                  [](PipelinePhase* CurPhase)
                                  {
                                      return CurPhase->Flags.Test(EPipelineFlags::Executing);
                                  }) |
                              views::transform(
                                  [](PipelinePhase* CurPhase)
                                  {
                                      return CurPhase->Task;
                                  }) |
                              ranges::to<std::vector>();

            for (auto CurPhase : Phase)
            {
                CurPhase->Flags.Set(EPipelineFlags::Executing, false);
            }

            Tasks.reserve(PhaseTasks.size());
            Tasks.insert(Tasks.end(), PhaseTasks.begin(), PhaseTasks.end());
        }

        cppcoro::sync_wait(cppcoro::when_all(Tasks));
    }

    void EnginePipeline::SetThreadCount(
        size_t ThreadCount)
    {
    }

    void EnginePipeline::SetPhaseEnable(
        const StringU8& PhaseName,
        bool            Enabled)
    {
        auto& Phase = m_Phases.find(PhaseName)->second;
        // Phase.Flags.Set(EPipelineFlags::Disabled, !Enabled);
    }

    void EnginePipeline::SetPhaseParallelize(
        const StringU8& PhaseName,
        bool            Parallelize)
    {
        auto& Phase = m_Phases.find(PhaseName)->second;
        // Phase.Flags.Set(EPipelineFlags::DontParallelize, !Parallelize);
    }
} // namespace Neon::Runtime