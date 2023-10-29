#include <EnginePCH.hpp>
#include <RenderGraph/SceneContext.hpp>
#include <RenderGraph/Storage.hpp>
#include <Runtime/GameLogic.hpp>

#include <Geometry/Frustum.hpp>

#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Scene/Component/CSG.hpp>

#include <RHI/GlobalBuffer.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/Material/Material.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::RG
{
    /// <summary>
    /// Create rendering query for specific component
    /// </summary>
    template<typename _ComponentTy, typename _RenderHandleTag = _ComponentTy>
    static auto CreateRenderingQuery()
    {
        return Scene::EntityWorld::Get()
            .query_builder<
                const Component::Transform,
                const _ComponentTy>()
            .with<Component::ActiveSceneEntity>()
            .in()
            .and_()
            .with<Scene::GPUTransformManager::RenderableHandle, _RenderHandleTag>()
            .in()
            .build();
    }

    /// <summary>
    /// Create rendering query for specific component
    /// </summary>
    template<typename _ComponentTy, typename _RenderHandleTag = _ComponentTy>
    static auto CreateRenderingRule()
    {
        return Scene::EntityWorld::Get()
            .rule_builder<
                const Component::Transform,
                const _ComponentTy>("Query")
            .with<Component::ActiveSceneEntity>()
            .in()
            .and_()
            .with<Scene::GPUTransformManager::RenderableHandle, _ComponentTy>()
            .in()
            .build();
    }

    //

    bool SceneContext::EntityInfo::CanRender(
        RenderType PassType,
        uint32_t   PassIndex) const
    {
        flecs::entity       Entity = Scene::EntityHandle(Id);
        Ptr<RHI::IMaterial> Material;

        switch (Type)
        {
        case EntityType::Mesh:
        case EntityType::CSG:
        {
            if (Type == EntityType::Mesh)
            {
                Material = Entity.get<Component::MeshInstance>()->Mesh.GetMaterial();
            }
            else
            {
                Material = Entity.get<Component::CSGBrush>()->Brush.GetMesh().GetMaterial();
            }
            break;
        }

        default:
        {
            std::unreachable();
        }
        }

        if (Material->IsCompute())
        {
            if (PassType != RenderType::RenderPass || PassIndex == 1)
            {
                return false;
            }
        }

        return true;
    }

    auto SceneContext::EntityInfo::GetRenderInfo(
        RHI::Views::Vertex& VtxView,
        RHI::Views::Index&  IdxView) const -> EntityRenderInfo
    {
        EntityRenderInfo RenderInfo;

        flecs::entity Entity = Scene::EntityHandle(Id);
        switch (Type)
        {
        case EntityType::Mesh:
        case EntityType::CSG:
        {
            const Mdl::Mesh* Mesh = nullptr;

            if (Type == EntityType::Mesh)
            {
                Mesh = &Entity.get<Component::MeshInstance>()->Mesh;

                RenderInfo.InstanceId = Entity.get<Scene::GPUTransformManager::RenderableHandle, Component::MeshInstance>()->InstanceId;
            }
            else
            {
                Mesh = &Entity.get<Component::CSGBrush>()->Brush.GetMesh();

                RenderInfo.InstanceId = Entity.get<Scene::GPUTransformManager::RenderableHandle, Component::CSGBrush>()->InstanceId;
            }

            auto& MeshModel = Mesh->GetModel();
            auto& MeshData  = Mesh->GetData();

            RenderInfo.Material = Mesh->GetMaterial();
            RenderInfo.Topology = MeshData.Topology;

            VtxView.Append<Mdl::MeshVertex>(MeshModel->GetVertexBuffer(), MeshData.VertexOffset, MeshData.VertexCount);

            if (MeshModel->HasSmallIndices())
            {
                IdxView = RHI::Views::IndexU16{ MeshModel->GetIndexBuffer(), MeshData.IndexOffset, MeshData.IndexCount };
            }
            else
            {
                IdxView = RHI::Views::IndexU32{ MeshModel->GetIndexBuffer(), MeshData.IndexOffset, MeshData.IndexCount };
            }

            break;
        }

        default:
        {
            std::unreachable();
        }
        }

        return RenderInfo;
    }

    //

    SceneContext::SceneContext(
        const GraphStorage& Storage) :
        m_Storage(Storage),
        m_MeshQuery(CreateRenderingQuery<Component::MeshInstance>()),
        m_CSGQuery(CreateRenderingQuery<Component::CSGBrush>())
    {
    }

    SceneContext::~SceneContext()
    {
        m_MeshQuery.destruct();
    }

    //

    void SceneContext::Update(
        const Component::Camera&    Camera,
        const Component::Transform& Transform)
    {
        m_EntityLists.clear();

        switch (Camera.Type)
        {
        case Component::CameraType::Perspective:
        {
            Geometry::Frustum Frustum(glm::transpose(m_Storage.GetFrameData().ProjectionInverse));
            Frustum.Transform(Transform);

            m_MeshQuery.iter(
                [&](flecs::iter&                   Iter,
                    const Component::Transform*    Transforms,
                    const Component::MeshInstance* Meshes)
                {
                    // TODO: Add instancing
                    for (size_t i : Iter)
                    {
                        auto& CurTransform = Transforms[i];
                        auto& CurMesh      = Meshes[i].Mesh;
                        auto  Box          = CurMesh.GetData().AABB;

                        Box.Transform(CurTransform);
                        if (Frustum.Contains(Box) != Geometry::ContainmentType::Disjoint)
                        {
                            auto& Material      = CurMesh.GetMaterial();
                            auto  PipelineState = Material->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass).get();

                            float Dist = glm::distance2(Transform.GetPosition(), CurTransform.GetPosition());
                            m_EntityLists[PipelineState].emplace(Iter.entity(i), Dist, EntityType::Mesh);
                        }
                    }
                });

            m_CSGQuery.iter(
                [&](flecs::iter&                Iter,
                    const Component::Transform* Transforms,
                    const Component::CSGBrush*  Brushes)
                {
                    for (size_t i : Iter)
                    {
                        auto& CurTransform = Transforms[i];
                        auto& CurMesh      = Brushes[i].Brush.GetMesh();
                        if (!CurMesh.GetModel())
                        {
                            continue;
                        }
                        auto Box = CurMesh.GetData().AABB;

                        Box.Transform(CurTransform);
                        if (Frustum.Contains(Box) != Geometry::ContainmentType::Disjoint)
                        {
                            auto& Material      = CurMesh.GetMaterial();
                            auto  PipelineState = Material->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass).get();

                            float Dist = glm::distance2(Transform.GetPosition(), CurTransform.GetPosition());
                            m_EntityLists[PipelineState].emplace(Iter.entity(i), Dist, EntityType::CSG);
                        }
                    }
                });
            break;
        }
        case Component::CameraType::Orthographic:
        {
            break;
        }

        default:
            std::unreachable();
        }
    }

    void SceneContext::Render(
        RHI::ICommandList*       CommandList,
        RenderType               Type,
        RHI::GpuDescriptorHandle OpaqueLightDataHandle,
        RHI::GpuDescriptorHandle TransparentLightDataHandle) const
    {
        if (m_EntityLists.empty())
        {
            return;
        }

        auto& GpuTransformManager = Runtime::GameLogic::Get()->GetGPUScene()->GetTransformManager();
        auto& GpuLightManager     = Runtime::GameLogic::Get()->GetGPUScene()->GetLightManager();

        uint32_t PassesCount = 1;

        std::array<RHI::IMaterial::PipelineVariant, 2> Passes;

#ifndef NEON_DIST
        static const std::array<StringU8, 2> PassNames{
            "OpaquePass",
            "TransparentPass"
        };
#endif

        switch (Type)
        {
        case RenderType::DepthPrepass:
            Passes[0] = RHI::IMaterial::PipelineVariant::DepthPrePass;
            break;
        case RenderType::RenderPass:
            Passes[0]   = RHI::IMaterial::PipelineVariant::RenderPass;
            Passes[1]   = RHI::IMaterial::PipelineVariant::RenderPassTransparent;
            PassesCount = 2;
            break;
        default:
            std::unreachable();
        }

        //

        bool RootSignatureWasBound[2]{};
        auto BindRootSignatureOnce = [&](bool IsDirect)
        {
            auto Index = IsDirect ? 0 : 1;
            if (RootSignatureWasBound[Index])
            {
                return;
            }

            RootSignatureWasBound[Index] = true;
            CommandList->BindMaterialParameters(IsDirect, m_Storage.GetFrameDataHandle());
        };

        RHI::GpuDescriptorHandle LightWasBound[2]{};
        auto                     BindLightOnce = [&](
                                 bool IsTransparent,
                                 bool IsDirect)
        {
            auto  Index           = IsDirect ? 0 : 1;
            auto& LightDataHandle = IsTransparent ? TransparentLightDataHandle : OpaqueLightDataHandle;
            if (LightWasBound[Index] == LightDataHandle)
            {
                return;
            }

            if (LightWasBound[Index] = LightDataHandle)
            {
                CommandList->SetDescriptorTable(
                    IsDirect,
                    uint32_t(RHI::RSCommon::MaterialRS::LightData),
                    LightDataHandle);
            }
        };

        RHI::IPipelineState* CurrentPipelineState  = nullptr;
        auto                 BindPipelineStateOnce = [&](const Ptr<RHI::IPipelineState>& PipelineState)
        {
            if (CurrentPipelineState != PipelineState.get())
            {
                CurrentPipelineState = PipelineState.get();
                CommandList->SetPipelineState(PipelineState);
            }
        };

        //

        // TODO: A DescriptorTable batcher to prevent reallocation of resources views

        for (uint32_t Pass = 0; Pass < PassesCount; Pass++)
        {
#ifndef NEON_DIST
            if (Type != RenderType::DepthPrepass)
            {
                CommandList->BeginEvent(PassNames[Pass]);
            }
#endif

            for (auto& EntityList : m_EntityLists | std::views::values)
            {
                // Ignore compute pipeline state on transparent / depthprepass
                if (!EntityList.begin()->CanRender(Type, Pass))
                {
                    continue;
                }

                for (auto& Info : EntityList)
                {
                    RHI::Views::Vertex VtxView;
                    RHI::Views::Index  IdxView;

                    auto RenderInfo = Info.GetRenderInfo(VtxView, IdxView);

                    // Pass == 0 opaque materials
                    // Pass == 1 transparent materials
                    if ((RenderInfo.Material->IsTransparent() ? 1 : 0) != Pass)
                    {
                        continue;
                    }

                    BindRootSignatureOnce(!RenderInfo.Material->IsCompute());
                    BindLightOnce(RenderInfo.Material->IsTransparent(), !RenderInfo.Material->IsCompute());
                    BindPipelineStateOnce(RenderInfo.Material->GetPipelineState(Passes[Pass]));

                    // Update shared params
                    {
                        // Update PerInstanceData
                        {
                            CommandList->SetResourceView(
                                !RenderInfo.Material->IsCompute(),
                                RHI::CstResourceViewType::Srv,
                                uint32_t(RHI::RSCommon::MaterialRS::InstanceData),
                                GpuTransformManager.GetInstanceHandle(RenderInfo.InstanceId));
                        }

                        // Update Local and shared data
                        {
                            RenderInfo.Material->ReallocateShared();
                            RenderInfo.Material->ReallocateLocal();

                            CommandList->SetResourceView(
                                !RenderInfo.Material->IsCompute(),
                                RHI::CstResourceViewType::Cbv,
                                uint32_t(RHI::RSCommon::MaterialRS::SharedData),
                                RenderInfo.Material->GetSharedBlock());

                            CommandList->SetResourceView(
                                !RenderInfo.Material->IsCompute(),
                                RHI::CstResourceViewType::Srv,
                                uint32_t(RHI::RSCommon::MaterialRS::LocalData),
                                RenderInfo.Material->GetLocalBlock());
                        }
                    }

                    CommandList->SetPrimitiveTopology(RenderInfo.Topology);
                    CommandList->SetIndexBuffer(IdxView);
                    CommandList->SetVertexBuffer(0, VtxView);
                    CommandList->Draw(RHI::DrawIndexArgs{ .IndexCountPerInstance = uint32_t(IdxView.Get().Size / (IdxView.Get().Is32Bit ? sizeof(uint32_t) : sizeof(uint16_t))) });
                }
            }

#ifndef NEON_DIST
            if (Type != RenderType::DepthPrepass)
            {
                CommandList->EndEvent();
            }
#endif
        }
    }

    RHI::CpuDescriptorHandle SceneContext::GetLightsResourceView() const noexcept
    {
        auto& GpuLightManager = Runtime::GameLogic::Get()->GetGPUScene()->GetLightManager();
        return GpuLightManager.GetInstancesView();
    }

    uint32_t SceneContext::GetLightsCount() const noexcept
    {
        auto& GpuLightManager = Runtime::GameLogic::Get()->GetGPUScene()->GetLightManager();
        return GpuLightManager.GetInstancesCount();
    }
} // namespace Neon::RG