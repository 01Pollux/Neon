#include <EnginePCH.hpp>
#include <Scene/GPU/TransformManager.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Scene/Component/CSG.hpp>

#include <RHI/GlobalBuffer.hpp>

namespace Neon::Scene
{
    template<typename _ComponentTy, typename _RenderHandleTag = _ComponentTy>
    static void HandleTransformInstance(
        GPUTransformManager* Manager)
    {
        using RenderableHandle = GPUTransformManager::RenderableHandle;
        using InstanceData     = GPUTransformManager::InstanceData;

        auto HandleInstanceCallback = [Manager](flecs::iter& Iter, const Component::Transform* Transforms)
        {
            for (size_t i : Iter)
            {
                auto& Transform = Transforms[i];
                auto  Entity    = Iter.entity(i);

                auto Handle = Entity.get_mut<RenderableHandle, _RenderHandleTag>();
                if (Iter.event() == flecs::OnSet)
                {
                    InstanceData* Data = nullptr;
                    if (Handle->Valid()) [[likely]]
                    {
                        Data = Manager->GetInstanceData(Handle->InstanceId);
                    }
                    else
                    {
                        Handle->InstanceId = Manager->AddInstance(&Data);
                    }
                    Data->World = Transform.ToMat4x4Transposed();
                    Entity.modified<RenderableHandle, _RenderHandleTag>();
                }
                else
                {
                    if (Handle->Valid()) [[likely]]
                    {
                        Manager->RemoveInstance(Handle->InstanceId);
                    }
                    Entity.remove<RenderableHandle, _RenderHandleTag>();
                }
            }
        };

        EntityWorld::Get()
            .observer<const Component::Transform>()
            .with<_ComponentTy>()
            .in()
            .event(flecs::OnSet)
            .event(flecs::OnRemove)
            .iter(HandleInstanceCallback);
    }

    GPUTransformManager::GPUTransformManager()
    {
        // Register the renderable handle component.
        EntityWorld::Get()
            .component<RenderableHandle>("_RenderableHandle");

        // Create observer for the transform and renderable components.
        HandleTransformInstance<Component::MeshInstance>(this);
        HandleTransformInstance<Component::CSGBrush>(this);
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
        RHI::UBufferPoolHandle TempBuffer(SizeOfInstanceData, AlignOfInstanceData, RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);
        TempBuffer.AsUpload().Write(TempBuffer.Offset, GetInstanceData(InstanceId), SizeOfInstanceData);
        return TempBuffer.GetGpuHandle();
    }
} // namespace Neon::Scene