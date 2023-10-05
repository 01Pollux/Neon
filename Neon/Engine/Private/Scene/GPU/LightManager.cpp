#include <EnginePCH.hpp>
#include <Scene/GPU/LightManager.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Light.hpp>

#include <RHI/Material/Material.hpp>

namespace Neon::Scene
{
    struct LightHandle
    {
        constexpr operator bool() const noexcept
        {
            return InstanceId != std::numeric_limits<uint32_t>::max();
        }

        /// <summary>
        /// Instance ID of the light in GPULightManager.
        /// </summary>
        uint32_t InstanceId = std::numeric_limits<uint32_t>::max();
    };

    GPULightManager::GPULightManager() :
        m_LightsBuffer(RHI::IGpuResource::Create(
            RHI::ResourceDesc::Buffer(
                MaxLightsInScene * SizeOfInstanceData,
                {},
                RHI::GraphicsBufferType::Upload),
            {
#ifndef NEON_DIST
                .Name = STR("InstanceLightBuffer"),
#endif
                .InitialState = RHI::IGpuResource::DefaultUploadResourceState })),
        m_LightsInScene(MaxLightsInScene),
        m_LightsBufferPtr(m_LightsBuffer->Map<InstanceData>())
    {
        // Create observer for the transform and renderable components.
        EntityWorld::Get()
            .observer<const Component::Transform>()
            .with<Component::Light>()
            .in()
            .event(flecs::OnSet)
            .event(flecs::OnRemove)
            .each(
                [this](flecs::iter& Iter, size_t Idx, const Component::Transform& Transform)
                {
                    auto Entity = Iter.entity(Idx);
                    auto Handle = Entity.get_mut<LightHandle>();
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
                    }
                    else
                    {
                        if (*Handle) [[likely]]
                        {
                            this->RemoveInstance(Handle->InstanceId);
                        }
                        Entity.remove<LightHandle>();
                    }
                });
    }

    uint32_t GPULightManager::AddInstance(
        InstanceData** InstanceData)
    {
        auto Id = uint32_t(m_LightsInScene.Allocate(1));
        if (!Id)
        {
            Id = InvalidInstanceId;
        }
        else
        {
            if (InstanceData)
            {
                *InstanceData = GetInstanceData(Id);
            }
        }
        return Id;
    }

    void GPULightManager::RemoveInstance(
        uint32_t InstanceId)
    {
        m_LightsInScene.Free({ .Offset = InstanceId, .Size = 1 });
    }

    auto GPULightManager::GetInstanceData(
        uint32_t InstanceId) -> InstanceData*
    {
        return m_LightsBufferPtr + InstanceId;
    }

    auto GPULightManager::GetInstanceData(
        uint32_t InstanceId) const -> const InstanceData*
    {
        return m_LightsBufferPtr + InstanceId;
    }

    RHI::GpuResourceHandle GPULightManager::GetInstanceHandle(
        uint32_t InstanceId) const
    {
        return m_LightsBuffer->GetHandle(InstanceId * SizeOfInstanceData);
    }
} // namespace Neon::Scene