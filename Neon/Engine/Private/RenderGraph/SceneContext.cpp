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
            Scene::EntityWorld()
                .Get()
                .query<
                    const Scene::Component::Transform,
                    const Scene::Component::MeshInstance,
                    const Scene::Component::Renderable>())

    {
    }

    void SceneContext::Update()
    {
        UpdateInstances();
    }

    void SceneContext::Render(
        RHI::ICommandList* CommandList,
        RenderType         Type) const
    {
    }

    void SceneContext::UpdateInstances()
    {
        if (!m_MeshQuery.changed())
        {
            return;
        }

        m_InstanceIds.clear();

        m_MeshQuery.iter(
            [this](flecs::iter& Iter,
                   const Scene::Component::Transform*,
                   const Scene::Component::MeshInstance* MeshTable,
                   const Scene::Component::Renderable*   RenderableTable)
            {
                for (size_t Idx : Iter)
                {
                    auto& Mesh       = MeshTable[Idx].Mesh;
                    auto& Renderable = RenderableTable[Idx];

                    auto& MeshData      = Mesh.GetData();
                    auto& MeshMaterial  = Mesh.GetModel()->GetMaterial(MeshData.MaterialIndex);
                    auto& PipelineState = MeshMaterial->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass);

                    m_InstanceIds[PipelineState].emplace_back(Renderable.GetInstanceId());
                }
            });
    }
} // namespace Neon::RG