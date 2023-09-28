#include <EnginePCH.hpp>
#include <Scene/GPUScene.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Renderable.hpp>
#include <Scene/Component/Transform.hpp>

#include <Log/Logger.hpp>

namespace Neon::Scene
{
    GPUScene::InstanceBufferPage::InstanceBufferPage(
        size_t PageIndex) :
        Instances(RHI::IGpuResource::Create(
            RHI::ResourceDesc::Buffer(
                SizeOfPage * SizeOfInstanceData,
                {},
                RHI::GraphicsBufferType::Upload),
            {
#ifndef NEON_DIST
                .Name = StringUtils::Format(STR("InstanceBufferPage_{}"), PageIndex).c_str(),
#endif
                .InitialState = RHI::IGpuResource::DefaultUploadResourceState })),
        Allocator(SizeOfPage),
        MappedInstances(Instances->Map<InstanceData>())
    {
    }

    //

    GPUScene::GPUScene()
    {
        EntityWorld::Get()
            .observer<Component::Transform, Component::Renderable>()
            .term_at(1)
            .in()
            .term_at(2)
            .inout()
            .event(flecs::OnSet)
            .event(flecs::OnRemove)
            .each(
                [this](flecs::iter& Iter, size_t Idx, const Component::Transform& Transform, Component::Renderable& Renderable)
                {
                    if (Renderable)
                    {
                        this->RemoveInstance(Renderable.InstanceId);
                    }
                    if (Iter.event() == flecs::OnSet)
                    {
                        InstanceData* Data    = nullptr;
                        Renderable.InstanceId = this->AddInstance(&Data);
                        Data->World           = Transform.World.ToMat4x4Transposed();
                    }
                });
    }

    uint32_t GPUScene::AddInstance(
        InstanceData** InstanceData)
    {
        uint32_t InstanceId = InvalidInstanceId;

        for (uint32_t i = 0; i < uint32_t(m_PagesInstances.size()); i++)
        {
            auto& Page = m_PagesInstances[i];
            if (auto Instance = Page.Allocator.Allocate(1))
            {
                InstanceId = uint32_t(i << 16) | uint32_t(Instance.Offset);
                if (InstanceData)
                {
                    *InstanceData = Page.MappedInstances + Instance.Offset;
                }
            }
        }

        if (InstanceId == InvalidInstanceId)
        {
            size_t Offset = m_PagesInstances.size();
            NEON_ASSERT(Offset < NumberOfPages, "No more space for instances.");
            auto& Page     = m_PagesInstances.emplace_back(Offset);
            auto  Instance = Page.Allocator.Allocate(1);
            InstanceId     = uint32_t(Offset << 16) | uint32_t(Instance.Offset);
            if (InstanceData)
            {
                *InstanceData = Page.MappedInstances + Instance.Offset;
            }
        }

        return InstanceId;
    }

    void GPUScene::RemoveInstance(
        uint32_t InstanceId)
    {
        uint32_t PageIndex = InstanceId >> 16;
        uint32_t Offset    = InstanceId & 0xFFFF;
        m_PagesInstances[PageIndex].Allocator.Free({ .Offset = Offset, .Size = 1 });
    }

    auto GPUScene::GetInstanceData(
        uint32_t InstanceId) -> InstanceData*
    {
        uint32_t PageIndex = InstanceId >> 16;
        uint32_t Offset    = InstanceId & 0xFFFF;
        return m_PagesInstances[PageIndex].MappedInstances + Offset;
    }

    auto GPUScene::GetInstanceData(
        uint32_t InstanceId) const -> const InstanceData*
    {
        uint32_t PageIndex = InstanceId >> 16;
        uint32_t Offset    = InstanceId & 0xFFFF;
        return m_PagesInstances[PageIndex].MappedInstances + Offset;
    }
} // namespace Neon::Scene