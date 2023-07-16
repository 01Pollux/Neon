#include <EnginePCH.hpp>
#include <Runtime/PipelineBuilder.hpp>

#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    auto EnginePipelineBuilder::NewPhase(
        const StringU8& PhaseName) -> PhaseRef
    {
        auto [Iter, Success] = m_Phases.emplace(PhaseName, PipelinePhase{ .Name = PhaseName });
        NEON_ASSERT(Success, "Pipeline phase '{}' already exists", PhaseName);
        return PhaseRef(Iter->second);
    }

    auto EnginePipelineBuilder::GetPhase(
        const StringU8& PhaseName) -> PhaseRef
    {
        auto Iter = m_Phases.find(PhaseName);
        NEON_ASSERT(Iter != m_Phases.end(), "Pipeline phase '{}' does not exist", PhaseName);
        return PhaseRef(Iter->second);
    }

    void EnginePipelineBuilder::PhaseRef::DependsOn(
        PhaseRef& Phase)
    {
        // Detect circular dependencies
        auto PendingPhases = this->m_Phase.DependentNodes;
        while (!PendingPhases.empty())
        {
            auto CheckPhase = PendingPhases.back();
            PendingPhases.pop_back();
            NEON_VALIDATE(CheckPhase != &Phase.m_Phase, "Circular dependency detected between phases '{}' and '{}'", this->m_Phase.Name, Phase.m_Phase.Name);

            PendingPhases.insert_range(PendingPhases.end(), CheckPhase->DependentNodes);
        }

        Phase.m_Phase.DependentNodes.emplace_back(&m_Phase);
        this->m_Phase.DependenciesCount++;
    }
} // namespace Neon::Runtime