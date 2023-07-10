#pragma once

#include <Utils/Signal.hpp>
#include <Core/BitMask.hpp>
#include <Runtime/PipelineBuilder.hpp>

#include <cppcoro/task.hpp>
#include <cppcoro/shared_task.hpp>
#include <cppcoro/static_thread_pool.hpp>

namespace Neon::Runtime
{
    enum class EPipelineFlags : uint8_t
    {
        /// <summary>
        /// Pipeline is diabled and will not be executed
        /// </summary>
        Disabled,

        /// <summary>
        /// Pipeline won't executed in parallel
        /// </summary>
        DontParallelize,

        /// <summary>
        /// Pipeline is currently executing
        /// </summary>
        Executing,

        _Last_Enum
    };
    using MPipelineFlags = Bitmask<EPipelineFlags>;

    class EnginePipeline
    {
    public:
        EnginePipeline(
            EnginePipelineBuilder Builder);

        /// <summary>
        /// Execute the phases in the pipeline
        /// </summary>
        void BeginDispatch();

        /// <summary>
        /// Execute the phases in the pipeline
        /// </summary>
        void EndDispatch();

        /// <summary>
        /// Enable or disable parallelization for the pipeline
        /// </summary>
        void SetThreadCount(
            size_t ThreadCount);

    public:
        /// <summary>
        /// Enable or disable a phase in the pipeline
        /// </summary>
        void SetPhaseEnable(
            const StringU8& PhaseName,
            bool            Enabled);

        /// <summary>
        /// Enable or disable parallelization for a phase in the pipeline
        /// </summary>
        void SetPhaseParallelize(
            const StringU8& PhaseName,
            bool            Parallelize);

    public:
        /// <summary>
        /// Attach callback to the phase and return signal managed handle
        /// </summary>
        template<std::invocable _FnTy>
        Utils::SignalHandle<> AttachUnique(
            const StringU8& PhaseName,
            _FnTy&&         Callback)
        {
            auto& Phase = m_Phases.find(PhaseName)->second;

            std::scoped_lock Lock(Phase.Mutex);
            return Utils::SignalHandle(Phase, std::forward<_FnTy>(Callback));
        }

        /// <summary>
        /// Attach callback to the phase
        /// </summary>
        template<std::invocable _FnTy>
        uint64_t Attach(
            const StringU8& PhaseName,
            _FnTy&&         Callback)
        {
            auto& Phase = m_Phases.find(PhaseName)->second;

            std::scoped_lock Lock(Phase.Mutex);
            return Phase.Signal.Listen(std::forward<_FnTy>(Callback));
        }

        /// <summary>
        /// Detach callback from the phase
        /// </summary>
        void Detach(
            const StringU8& PhaseName,
            uint64_t        Id)
        {
            auto& Phase = m_Phases.find(PhaseName)->second;

            std::scoped_lock Lock(Phase.Mutex);
            return Phase.Signal.Drop(Id);
        }

    private:
        struct PipelinePhase;

        using PhaseMapType        = std::map<StringU8, PipelinePhase>;
        using ParentPhaseListType = std::vector<PipelinePhase*>;
        using PhaseLevelListType  = std::vector<std::vector<PipelinePhase*>>;

        struct PipelinePhase
        {
            ParentPhaseListType    Parents;
            Utils::Signal<>        Signal;
            std::mutex             Mutex;
            cppcoro::shared_task<> Task;
            MPipelineFlags         Flags;
        };

    private:
        PhaseMapType       m_Phases;
        PhaseLevelListType m_Levels;

        cppcoro::static_thread_pool m_ThreadPool;
        std::vector<PipelinePhase*> m_NonAsyncPhases;
    };
} // namespace Neon::Runtime