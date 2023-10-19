#include <EnginePCH.hpp>
#include <Scene/GPU/TransformManager.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>

#include <RHI/Material/Material.hpp>

namespace Neon::Scene
{
    GPUTransformManager::GPUTransformManager()
    {
        // Register the renderable handle component.
        EntityWorld::Get()
            .component<RenderableHandle>("_RenderableHandle");

        // Register pipeline group.
        EntityWorld::Get()
            .component<PipelineGroup>("_PipelineGroup");

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
                    auto Handle = Entity.get_mut<RenderableHandle>();
                    if (Iter.event() == flecs::OnSet)
                    {
                        InstanceData* Data = nullptr;
                        if (*Handle) [[likely]]
                        {
                            Data = this->GetInstanceData(Handle->InstanceId);
                        }
                        else
                        {
                            Handle->InstanceId = this->AddInstance(&Data);
                        }
                        Data->World = Transform.World.ToMat4x4Transposed();
                    }
                    else
                    {
                        if (*Handle) [[likely]]
                        {
                            this->RemoveInstance(Handle->InstanceId);
                        }
                        Entity.remove<RenderableHandle>();
                    }
                });

        // Create observer for the transform and renderable components.
        EntityWorld::Get()
            .observer<const Component::Transform, const Component::MeshInstance>()
            .event(flecs::OnSet)
            .event(flecs::OnRemove)
            .each(
                [this](flecs::iter& Iter, size_t Idx, const Component::Transform&, const Component::MeshInstance& Instance)
                {
                    auto Entity = Iter.entity(Idx);
                    if (Iter.event() == flecs::OnSet)
                    {
                        uint32_t MaterialIndex = Instance.Mesh.GetData().MaterialIndex;
                        auto&    Material      = Instance.Mesh.GetModel()->GetMaterial(MaterialIndex);
                        auto     PipelineState = Material->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass).get();
                        Entity.add<PipelineGroup>(std::bit_cast<uint64_t>(PipelineState));
                    }
                    else
                    {
                        Entity.remove<PipelineGroup>();
                    }
                });

        //
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