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
                        if (Handle->Valid()) [[likely]]
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
                        if (Handle->Valid()) [[likely]]
                        {
                            this->RemoveInstance(Handle->InstanceId);
                        }
                        Entity.remove<RenderableHandle>();
                    }
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