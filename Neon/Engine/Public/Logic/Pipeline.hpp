#pragma once

#include <Utils/Signal.hpp>
#include <Core/String.hpp>
#include <Core/BitMask.hpp>
#include <map>

namespace Neon
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

        _Last_Enum
    };
    using MPipelineFlags = Bitmask<EPipelineFlags>;

    class PipelineBuilder;

    class Pipeline
    {
    public:
        Pipeline(
            PipelineBuilder Builder);

        /// <summary>
        /// Begin the execution of the phases.
        /// </summary>
        void BeginPhases();

        /// <summary>
        /// End the execution of the phases.
        /// </summary>
        void EndPhases();

        /// <summary>
        /// Set the number of threads to use for parallelization
        /// </summary>
        /// <param name="ThreadCount"></param>
        void SetThreadCount(
            uint32_t ThreadCount);

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
            return Phase.Signal.Drop(Id);
        }

    private:
        struct PipelinePhase
        {
            Utils::Signal<> Signal;
            MPipelineFlags  Flags;
        };
        using PhaseMap       = std::map<StringU8, PipelinePhase>;
        using PhaseLevelList = std::vector<std::vector<PipelinePhase*>>;

    private:
        PhaseMap       m_Phases;
        PhaseLevelList m_Levels;
        std::jthread   m_ExecutionThread;
        std::mutex     m_ExecuteMutex;
        uint32_t       m_ThreadCount;
    };
} // namespace Neon