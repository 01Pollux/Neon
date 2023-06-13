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
    };
    using MPipelineFlags = Bitmask<EPipelineFlags>;

    class Pipeline
    {
    public:
        Pipeline();

        /// <summary>
        /// Dispatch the phases.
        /// </summary>
        void Dispatch();

        /// <summary>
        /// Set the number of threads to use for parallelization
        /// </summary>
        /// <param name="ThreadCount"></param>
        void SetThreadCount(
            uint16_t ThreadCount);

    public:
        /// <summary>
        /// Enable or disable a phase in the pipeline
        /// </summary>
        void SetPhaseEnable(
            const StringU8& PhaseName,
            bool            State);

        /// <summary>
        /// Enable or disable parallelization for a phase in the pipeline
        /// </summary>
        void SetPhaseParallelize(
            const StringU8& PhaseName,
            bool            State);

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
        struct Phase
        {
            Utils::Signal<void()> Signal;
        };
        using PhaseMap       = std::map<StringU8, Phase>;
        using PhaseLevelList = std::vector<std::vector<PhaseMap::iterator>>;

    private:
        PhaseMap       m_Phases;
        PhaseLevelList m_Levels;
        uint32_t       m_ThreadCount;
    };
} // namespace Neon