#include <EnginePCH.hpp>
#include <Scene/GPU/TransformManager.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Scene/Component/Renderable.hpp>

#include <RHI/Material/Material.hpp>

namespace Neon::Scene
{
    GPUTransformManager::GPUTransformManager()
    {
        // Create observer for the transform and renderable components.
        EntityWorld::Get()
            .observer<const Component::Transform>()
            .with<Component::MeshInstance>()
            .in()
            .event(flecs::OnSet)
            .event(flecs::OnRemove)
            .each(
                [this](flecs::iter& Iter, size_t Idx, const Component::Transform& Transform)
                {
                    auto Entity = Iter.entity(Idx);
                    if (Iter.event() == flecs::OnSet)
                    {
                        auto Renderable = Entity.get_mut<Component::Renderable>();

                        InstanceData* Data = nullptr;
                        if (*Renderable) [[likely]]
                        {
                            Data = this->GetInstanceData(Renderable->InstanceId);
                        }
                        else
                        {
                            Renderable->InstanceId = this->AddInstance(&Data);
                        }
                        Data->World = Transform.World.ToMat4x4Transposed();
                    }
                    else
                    {
                        Entity.remove<Component::Renderable>();
                    }
                });

        //

        // Create mesh query
        auto MeshQuery =
            EntityWorld::Get()
                .query_builder<
                    const Scene::Component::MeshInstance,
                    const Scene::Component::Renderable>()
                .with<Scene::Component::ActiveSceneEntity>()
                .in()
                .build();

        // Update the mesh ever frame if needed
        EntityWorld::Get()
            .system("MeshQueryUpdate")
            .kind(flecs::PreUpdate)
            .iter(
                [this, MeshQuery](flecs::iter& Iter)
                {
                    if (!MeshQuery.changed())
                    {
                        return;
                    }

                    m_MeshInstanceIds.clear();
                    m_Meshes.clear();

                    MeshQuery.iter(
                        [this](flecs::iter&                          Iter,
                               const Scene::Component::MeshInstance* MeshInstances,
                               const Scene::Component::Renderable*   Renderables)
                        {
                            for (size_t Idx : Iter)
                            {
                                auto& Mesh       = MeshInstances[Idx].Mesh;
                                auto& Renderable = Renderables[Idx];

                                auto& MeshData      = Mesh.GetData();
                                auto& MeshMaterial  = Mesh.GetModel()->GetMaterial(MeshData.MaterialIndex);
                                auto& PipelineState = MeshMaterial->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass);

                                m_MeshInstanceIds[PipelineState.get()].emplace_back(Renderable.GetInstanceId());
                                m_Meshes[Renderable.GetInstanceId()] = &Mesh;
                            }
                        });
                });
    }

    uint32_t GPUTransformManager::AddInstance(
        InstanceData** InstanceData)
    {
        return m_PagesInstances.AddInstance(InstanceData);
    }

    void GPUTransformManager::RemoveInstance(
        uint32_t InstanceId)
    {
        m_PagesInstances.RemoveInstance(InstanceId);
    }

    auto GPUTransformManager::GetInstanceData(
        uint32_t InstanceId) -> InstanceData*
    {
        return m_PagesInstances.GetInstanceData(InstanceId);
    }

    auto GPUTransformManager::GetInstanceData(
        uint32_t InstanceId) const -> const InstanceData*
    {
        return m_PagesInstances.GetInstanceData(InstanceId);
    }

    RHI::GpuResourceHandle GPUTransformManager::GetInstanceHandle(
        uint32_t InstanceId) const
    {
        return m_PagesInstances.GetInstanceHandle(InstanceId);
    }
} // namespace Neon::Scene