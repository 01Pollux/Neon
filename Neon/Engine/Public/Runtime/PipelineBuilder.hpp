#pragma once

#include <Core/String.hpp>
#include <vector>
#include <map>

namespace Neon::Runtime
{
    class EnginePipeline;

    class EnginePipelineBuilder
    {
        struct PipelinePhase
        {
            StringU8                    Name;
            std::vector<PipelinePhase*> DependentNodes;
            size_t                      DependenciesCount = 0;
        };

        using PhasesMapType = std::map<StringU8, PipelinePhase>;

        friend class EnginePipeline;

    public:
        class PhaseRef;

        /// <summary>
        /// Append a new phase to the pipeline.
        /// </summary>
        PhaseRef NewPhase(
            const StringU8& PhaseName);

        /// <summary>
        /// Get a phase by name.
        /// </summary>
        PhaseRef GetPhase(
            const StringU8& PhaseName);

    private:
        PhasesMapType m_Phases;
    };

    class EnginePipelineBuilder::PhaseRef
    {
        friend class EnginePipelineBuilder;

    public:
        /// <summary>
        /// Add a dependency to the phase.
        /// </summary>
        void DependsOn(
            PhaseRef& Phase);

        /// <summary>
        /// Add a dependency to the phase.
        /// </summary>
        template<typename... _Args>
            requires std::same_as<PhaseRef, std::common_type_t<_Args...>>
        void Then(
            _Args&... Phase)
        {
            (Phase.DependsOn(*this), ...);
        }

        /// <summary>
        /// Add a dependency to the phase.
        /// </summary>
        PhaseRef& Then(
            PhaseRef& Phase)
        {
            Phase.DependsOn(*this);
            return Phase;
        }

    protected:
        PhaseRef(
            PipelinePhase& Iter) :
            m_Phase(Iter)
        {
        }

    private:
        PipelinePhase& m_Phase;
    };
} // namespace Neon::Runtime