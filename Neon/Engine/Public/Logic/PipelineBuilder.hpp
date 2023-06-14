#pragma once

#include <Core/String.hpp>
#include <vector>
#include <map>

namespace Neon
{
    class Pipeline;

    class PipelineBuilder
    {
        struct PipelinePhase
        {
            StringU8                    Name;
            std::vector<PipelinePhase*> DependentNodes;
            size_t                      DependenciesCount = 0;
        };

        using PhasesMapType = std::map<StringU8, PipelinePhase>;

        friend class Pipeline;

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

    class PipelineBuilder::PhaseRef
    {
        friend class PipelineBuilder;

    public:
        /// <summary>
        /// Add a dependency to the phase.
        /// </summary>
        void DependsOn(
            PhaseRef& Phase);

    protected:
        PhaseRef(
            PipelinePhase& Iter) :
            m_Phase(Iter)
        {
        }

    private:
        PipelinePhase& m_Phase;
    };
} // namespace Neon