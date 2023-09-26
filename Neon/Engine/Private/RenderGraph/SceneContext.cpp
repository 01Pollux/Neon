#include <EnginePCH.hpp>
#include <RenderGraph/SceneContext.hpp>
#include <Scene/EntityWorld.hpp>
#include <RHI/Material/Material.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::RG
{
    SceneContext::SceneContext(
        const GraphStorage& Storage) :
        m_Storage(Storage),
        m_MeshQuery(
            Scene::EntityWorld::Get()
                .query_builder<Component::Transform, Component::MeshInstance>()
                .with<Component::ActiveSceneEntity>()
                .term<Component::Transform>()
                .in()
                .term<Component::MeshInstance>()
                .in()
                // Order by material's pipeline state
                .order_by<Component::MeshInstance>(
                    [](flecs::entity_t                EntLhs,
                       const Component::MeshInstance* MeshLhs,
                       flecs::entity_t                EntRhs,
                       const Component::MeshInstance* MeshRhs) -> int32_t
                    {
                        auto& ModelLhs = MeshLhs->Mesh.GetModel();
                        auto& ModelRhs = MeshRhs->Mesh.GetModel();

                        auto MatIdxLhs = MeshLhs->Mesh.GetSubmesh().MaterialIndex;
                        auto MatIdxRhs = MeshRhs->Mesh.GetSubmesh().MaterialIndex;

                        auto& MaterialLhs = ModelLhs->GetMaterial(MatIdxLhs);
                        auto& MaterialRhs = ModelRhs->GetMaterial(MatIdxRhs);

                        auto& PipelineStateLhs = MaterialLhs->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass);
                        auto& PipelineStateRhs = MaterialRhs->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass);

                        if (PipelineStateLhs < PipelineStateRhs)
                        {
                            return -1;
                        }
                        else if (PipelineStateLhs > PipelineStateRhs)
                        {
                            return 1;
                        }
                        return 0;
                    })
                .build())
    {
    }

    void SceneContext::Render(
        RHI::ICommandList* CommandList,
        RenderType         Type) const
    {
        if (m_MeshQuery.is_true())
        {
        }
    }
} // namespace Neon::RG