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
            std::vector<PipelinePhase*> Dependencies;
            size_t                      DependenciesCount;
        };

        using PhasesMapType = std::map<StringU8, PipelinePhase>;

    public:
        class PhaseRef;

        /// <summary>
        /// Append a new phase to the pipeline.
        /// </summary>
        PhaseRef& NewPhase(
            StringU8 PhaseName);

        /// <summary>
        /// Get a phase by name.
        /// </summary>
        PhaseRef& GetPhase(
            const StringU8& PhaseName);

    private:
        PhasesMapType m_Phases;
    };

    class PipelineBuilder::PhaseRef
    {
        friend class PipelineBuilder;

    public:
    protected:
        PhaseRef(
            const PhasesMapType::iterator& Iter) :
            m_Phase(Iter)
        {
        }

    private:
        PhasesMapType::iterator m_Phase;
    };
} // namespace Neon