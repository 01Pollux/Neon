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

    SceneContext::SceneContext(
        const GraphStorage& Storage) :
        m_Storage(Storage),
        m_MeshQuery(CreateRenderingQuery<Component::MeshInstance>()),
        m_CSGRule(CreateRenderingRule<Component::CSGShape>())
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
                    for (size_t Index : Iter)
                    {
                        auto& CurTransform = Transforms[Index];
                        auto& CurMesh      = Meshes[Index].Mesh;
                        auto  Box          = CurMesh.GetData().AABB;

                        Box.Transform(CurTransform);
                        if (Frustum.Contains(Box) != Geometry::ContainmentType::Disjoint)
                        {
                            auto& Material      = CurMesh.GetMaterial();
                            auto  PipelineState = Material->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass).get();

                            float Dist = glm::distance2(Transform.GetPosition(), CurTransform.GetPosition());
                            m_EntityLists[PipelineState].emplace(Iter.entity(Index), Dist);
                        }
                    }
                });

            m_CSGRule.iter(
                [&](flecs::iter&                Iter,
                    const Component::Transform* Transforms,
                    const Component::CSGShape*  Shapes)
                {
                    for (size_t Index : Iter)
                    {
                        auto& CurTransform = Transforms[Index];
                        auto& CurShape     = Shapes[Index];
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

        size_t PassesCount = 1;

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

        for (size_t Pass = 0; Pass < PassesCount; Pass++)
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
                {
                    flecs::entity FirstEntity = Scene::EntityHandle(EntityList.begin()->Id);

                    if (auto MeshInstance = FirstEntity.get<Component::MeshInstance>())
                    {
                        auto& FirstMaterial = MeshInstance->Mesh.GetMaterial();
                        auto& PipelineState = FirstMaterial->GetPipelineState(Passes[Pass]);

                        if (FirstMaterial->IsCompute() && Type != RenderType::RenderPass || Pass == 1)
                        {
                            continue;
                        }
                    }
                }

                for (auto& [Index, Z] : EntityList)
                {
                    flecs::entity Entity = Scene::EntityHandle(Index);

                    auto& Mesh       = Entity.get<Component::MeshInstance>()->Mesh;
                    auto  InstanceId = Entity.get<Scene::GPUTransformManager::RenderableHandle, Component::MeshInstance>()->InstanceId;

                    auto& MeshMaterial = Mesh.GetMaterial();
                    auto& MeshModel    = Mesh.GetModel();
                    auto& MeshData     = Mesh.GetData();

                    // Pass == 0 opaque materials
                    // Pass == 1 transparent materials
                    if ((MeshMaterial->IsTransparent() ? 1 : 0) != Pass)
                    {
                        continue;
                    }

                    BindRootSignatureOnce(!MeshMaterial->IsCompute());
                    BindLightOnce(MeshMaterial->IsTransparent(), !MeshMaterial->IsCompute());
                    BindPipelineStateOnce(MeshMaterial->GetPipelineState(Passes[Pass]));

                    // Update shared params
                    {
                        // Update PerInstanceData
                        {
                            CommandList->SetResourceView(
                                !MeshMaterial->IsCompute(),
                                RHI::CstResourceViewType::Srv,
                                uint32_t(RHI::RSCommon::MaterialRS::InstanceData),
                                GpuTransformManager.GetInstanceHandle(InstanceId));
                        }

                        // Update Local and shared data
                        {
                            MeshMaterial->ReallocateShared();
                            MeshMaterial->ReallocateLocal();

                            CommandList->SetResourceView(
                                !MeshMaterial->IsCompute(),
                                RHI::CstResourceViewType::Cbv,
                                uint32_t(RHI::RSCommon::MaterialRS::SharedData),
                                MeshMaterial->GetSharedBlock());

                            CommandList->SetResourceView(
                                !MeshMaterial->IsCompute(),
                                RHI::CstResourceViewType::Srv,
                                uint32_t(RHI::RSCommon::MaterialRS::LocalData),
                                MeshMaterial->GetLocalBlock());
                        }
                    }

                    RHI::Views::Vertex VtxView;
                    VtxView.Append<Mdl::MeshVertex>(
                        MeshModel->GetVertexBuffer()->GetHandle(),
                        MeshData.VertexOffset,
                        MeshData.VertexCount);

                    RHI::Views::IndexU32 IdxView(
                        MeshModel->GetIndexBuffer()->GetHandle(),
                        MeshData.IndexOffset,
                        MeshData.IndexCount);

                    CommandList->SetPrimitiveTopology(MeshData.Topology);
                    CommandList->SetIndexBuffer(IdxView);
                    CommandList->SetVertexBuffer(0, VtxView);
                    CommandList->Draw(RHI::DrawIndexArgs{ .IndexCountPerInstance = MeshData.IndexCount });
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