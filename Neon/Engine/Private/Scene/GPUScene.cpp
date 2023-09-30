#include <EnginePCH.hpp>
#include <Scene/GPUScene.hpp>
#include <Scene/EntityWorld.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Scene/Component/Renderable.hpp>

#include <RHI/Material/Material.hpp>

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
        // Reserve at least one page.
        m_PagesInstances.emplace_back(0);

        // Create observer for the transform and renderable components.
        EntityWorld::Get()
            .observer<const Component::Transform, Component::Renderable>()
            .event(flecs::OnSet)
            .event(flecs::OnRemove)
            .each(
                [this](flecs::iter& Iter, size_t Idx, const Component::Transform& Transform, Component::Renderable& Renderable)
                {
                    auto o = Iter.entity(Idx).id();
                    if (Iter.event() == flecs::OnSet)
                    {
                        InstanceData* Data = nullptr;
                        if (Renderable) [[likely]]
                        {
                            Data = this->GetInstanceData(Renderable.GetInstanceId());
                        }
                        else
                        {
                            Renderable.InstanceId = this->AddInstance(&Data);
                        }
                        Data->World = Transform.World.ToMat4x4Transposed();
                    }
                    else if (Renderable)
                    {
                        this->RemoveInstance(Renderable.InstanceId);
                        Renderable = {};
                    }
                });

        //

        // Create mesh query
        auto MeshQuery =
            EntityWorld::Get()
                .query_builder<
                    const Scene::Component::MeshInstance,
                    const Scene::Component::Renderable>()
                .with<Scene::Component::Transform>()
                .in()
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

    RHI::GpuResourceHandle GPUScene::GetInstanceHandle(
        uint32_t InstanceId) const
    {
        uint32_t PageIndex = InstanceId >> 16;
        uint32_t Offset    = InstanceId & 0xFFFF;
        return m_PagesInstances[PageIndex].Instances->GetHandle(Offset * SizeOfInstanceData);
    }
} // namespace Neon::Scene