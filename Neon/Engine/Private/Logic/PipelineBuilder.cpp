#include <EnginePCH.hpp>
#include <Logic/PipelineBuilder.hpp>

#include <Log/Logger.hpp>

namespace Neon
{
    auto PipelineBuilder::NewPhase(
        const StringU8& PhaseName) -> PhaseRef
    {
        auto [Iter, Success] = m_Phases.emplace(PhaseName, PipelinePhase{ .Name = PhaseName });
        NEON_ASSERT(Success, "Pipeline phase '{}' already exists", PhaseName);
        return PhaseRef(Iter->second);
    }

    auto PipelineBuilder::GetPhase(
        const StringU8& PhaseName) -> PhaseRef
    {
        auto Iter = m_Phases.find(PhaseName);
        NEON_ASSERT(Iter != m_Phases.end(), "Pipeline phase '{}' does not exist", PhaseName);
        return PhaseRef(Iter->second);
    }

    void PipelineBuilder::PhaseRef::DependsOn(
        PhaseRef& Phase)
    {
        // Detect circular dependencies
        auto PendingPhases = m_Phase.DependentNodes;
        while (!PendingPhases.empty())
        {
            auto CheckPhase = PendingPhases.back();
            PendingPhases.pop_back();
            NEON_ASSERT(CheckPhase != &Phase.m_Phase, "Circular dependency detected between phases '{}' and '{}'", m_Phase.Name, Phase.m_Phase.Name);

            PendingPhases.insert_range(PendingPhases.end(), CheckPhase->DependentNodes);
        }

        Phase.m_Phase.DependentNodes.emplace_back(&m_Phase);
        this->m_Phase.DependenciesCount++;
    }
} // namespace Neon