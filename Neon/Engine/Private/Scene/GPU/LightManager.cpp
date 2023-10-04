#include <EnginePCH.hpp>
#include <Scene/GPU/LightManager.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Scene/Component/Renderable.hpp>

#include <RHI/Material/Material.hpp>

namespace Neon::Scene
{
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
    }

    uint32_t GPULightManager::AddInstance(
        InstanceData** InstanceData)
    {
        auto Id = m_LightsInScene.Allocate(1);
        if (!Id)
        {
            Id.Offset = InvalidInstanceId;
        }
        else
        {
            if (InstanceData)
            {
                *InstanceData = GetInstanceData(Id.Offset);
            }
        }
        return Id.Offset;
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