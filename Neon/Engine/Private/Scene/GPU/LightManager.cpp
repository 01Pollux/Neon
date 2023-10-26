#include <EnginePCH.hpp>
#include <Scene/GPU/LightManager.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Light.hpp>

#include <RHI/Material/Material.hpp>
#include <RHI/GlobalDescriptors.hpp>

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
            RHI::ResourceDesc::BufferUpload(
                MaxLightsInScene * SizeOfInstanceData,
                {}),
            {
#ifndef NEON_DIST
                .Name = STR("InstanceLightBuffer")
#endif
            })),
        m_LightsInScene(MaxLightsInScene),
        m_LightsBufferPtr(m_LightsBuffer->Map())
    {
        m_LightsView = RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);

        RHI::SRVDesc SrvDesc{
            .View = RHI::SRVDesc::Buffer{
                .Count        = MaxLightsInScene,
                .SizeOfStruct = SizeOfInstanceData }
        };
        m_LightsView->CreateShaderResourceView(
            m_LightsView.Offset,
            m_LightsBuffer.get(),
            &SrvDesc);

        // Create observer for the transform and renderable components.
        EntityWorld::Get()
            .observer<const Component::Transform>()
            .expr("[in] ActiveSceneEntity, [in] DirectionalLight || [in] PointLight || [in] SpotLight")
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

                        uint32_t Type  = 0;
                        uint32_t Flags = (Data->Flags & ~uint32_t(LightFlags::TypeMask));

                        if (auto DirectionalLight = Entity.get<Component::DirectionalLight>())
                        {
                            Data->Color = DirectionalLight->Color;
                            Flags |= uint32_t(LightFlags::Directional);
                        }
                        else if (auto PointLight = Entity.get<Component::PointLight>())
                        {
                            Data->Color               = PointLight->Color;
                            Data->Range               = PointLight->Range;
                            Data->Attenuation_Angle.x = PointLight->Attenuation;
                            Flags |= uint32_t(LightFlags::Point);
                        }
                        else if (auto SpotLight = Entity.get<Component::SpotLight>())
                        {
                            Data->Color               = SpotLight->Color;
                            Data->Range               = SpotLight->Range;
                            Data->Attenuation_Angle.x = SpotLight->Attenuation;
                            Data->Attenuation_Angle.y = SpotLight->Angle;
                            Data->Attenuation_Angle.z = SpotLight->AngleAttenuation;
                            Flags |= uint32_t(LightFlags::Spot);
                        }

                        Data->Flags     = Flags;
                        Data->Position  = Transform.GetPosition();
                        Data->Direction = Transform.GetLookDir();
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
    } // namespace Neon::Scene

    GPULightManager::~GPULightManager()
    {
        if (m_LightsView)
        {
            RHI::IStaticDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Free(m_LightsView);
            m_LightsView = {};
        }
    }

    uint32_t GPULightManager::AddInstance(
        InstanceData** OutData)
    {
        auto Id = m_LightsInScene.Allocate(1);
        if (!Id)
        {
            Id.Offset = InvalidInstanceId;
        }
        else
        {
            InstanceData* Data = GetInstanceData(uint32_t(Id.Offset));
            Data->Flags |= uint32_t(LightFlags::Enabled);
            if (OutData)
            {
                *OutData = Data;
            }
            ++m_InstancesCount;
        }
        return uint32_t(Id.Offset);
    }

    void GPULightManager::RemoveInstance(
        uint32_t InstanceId)
    {
        auto Data = GetInstanceData(InstanceId);
        Data->Flags &= ~uint32_t(LightFlags::Enabled);
        m_LightsInScene.Free({ .Offset = InstanceId, .Size = 1 });
        m_InstancesCount--;
    }

    auto GPULightManager::GetInstanceData(
        uint32_t InstanceId) -> InstanceData*
    {
        return std::bit_cast<InstanceData*>(m_LightsBufferPtr + InstanceId * SizeOfInstanceData);
    }

    auto GPULightManager::GetInstanceData(
        uint32_t InstanceId) const -> const InstanceData*
    {
        return std::bit_cast<const InstanceData*>(m_LightsBufferPtr + InstanceId * SizeOfInstanceData);
    }

    RHI::CpuDescriptorHandle GPULightManager::GetInstancesView() const
    {
        return m_LightsView.GetCpuHandle();
    }

    const Ptr<RHI::IGpuResource>& GPULightManager::GetInstancesBuffer() const
    {
        return m_LightsBuffer;
    }

    uint32_t GPULightManager::GetInstancesCount() const
    {
        return m_InstancesCount;
    }
} // namespace Neon::Scene