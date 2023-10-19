#include <EnginePCH.hpp>
#include <RenderGraph/SceneContext.hpp>
#include <RenderGraph/Storage.hpp>
#include <Runtime/GameLogic.hpp>

#include <Scene/EntityWorld.hpp>
#include <Scene/Component/Component.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Mdl/Mesh.hpp>

#include <RHI/GlobalBuffer.hpp>
#include <RHI/RootSignature.hpp>
#include <RHI/Material/Material.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::RG
{
    SceneContext::SceneContext(
        const GraphStorage& Storage) :
        m_Storage(Storage)
    {
        // So that we can sort by distance to camera, we will need access to the camera position
        m_TransformOrderer =
            [this](flecs::entity_t,
                   const void* LhsTransformPtr,
                   flecs::entity_t,
                   const void* RhsTransformPtr) -> int
        {
            auto LhsTransform = std::bit_cast<const Component::Transform*>(LhsTransformPtr);
            auto RhsTransform = std::bit_cast<const Component::Transform*>(RhsTransformPtr);

            auto&   WorldTransform = m_Storage.GetFrameData().World;
            Vector3 CameraPosition(WorldTransform[3][0], WorldTransform[3][1], WorldTransform[3][2]);

            float LhsDistance = glm::distance(CameraPosition, LhsTransform->World.GetPosition());
            float RhsDistance = glm::distance(CameraPosition, RhsTransform->World.GetPosition());

            return LhsDistance < RhsDistance ? -1 : 1;
        };

        // Create mesh query
        m_MeshQuery =
            Scene::EntityWorld::Get()
                .query_builder<
                    const Scene::GPUTransformManager::RenderableHandle,
                    const Component::Transform,
                    const Component::MeshInstance>()
                .with<Component::ActiveSceneEntity>()
                .group_by<Scene::GPUTransformManager::PipelineGroup>()
                .order_by(
                    Scene::EntityWorld::Get().component<Component::Transform>(),
                    m_TransformOrderer.target<QueryOrdererC>())
                .build();
    }

    SceneContext::~SceneContext()
    {
        m_MeshQuery.destruct();
    }

    //

    void SceneContext::Render(
        RHI::ICommandList*       CommandList,
        RenderType               Type,
        RHI::GpuDescriptorHandle OpaqueLightDataHandle,
        RHI::GpuDescriptorHandle TransparentLightDataHandle) const
    {
        if (!m_MeshQuery.is_true())
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

        bool RootSignatureWasBound[2]{};
        auto BindRootSignatureOnce = [&RootSignatureWasBound, CommandList, this](bool IsDirect)
        {
            auto Index = IsDirect ? 0 : 1;
            if (RootSignatureWasBound[Index])
            {
                return;
            }

            RootSignatureWasBound[Index] = true;
            CommandList->BindMaterialParameters(IsDirect, m_Storage.GetFrameDataHandle());
        };

        //

        RHI::GpuDescriptorHandle LastLightHandle[2];
        for (size_t Pass = 0; Pass < PassesCount; Pass++)
        {
#ifndef NEON_DIST
            if (Type != RenderType::DepthPrepass)
            {
                CommandList->BeginEvent(PassNames[Pass]);
            }
#endif

            m_MeshQuery.iter(
                [&](flecs::iter&                                        Iter,
                    const Scene::GPUTransformManager::RenderableHandle* Renderables,
                    const Component::Transform*                         Transforms,
                    const Component::MeshInstance*                      Meshes)
                {
                    auto PassType = Passes[Pass];

                    auto& FirstMaterial = Meshes->Mesh.GetModel()->GetMaterial(Meshes->Mesh.GetData().MaterialIndex);
                    if (FirstMaterial->IsCompute() && Type != RenderType::RenderPass)
                    {
                        return;
                    }

                    CommandList->SetPipelineState(FirstMaterial->GetPipelineState(PassType));

                    for (auto Index : Iter)
                    {
                        auto& Mesh       = Meshes[Index].Mesh;
                        auto  InstanceId = Renderables[Index].InstanceId;

                        auto& MeshData     = Mesh.GetData();
                        auto& MeshModel    = Mesh.GetModel();
                        auto& MeshMaterial = Mesh.GetModel()->GetMaterial(MeshData.MaterialIndex);

                        if (MeshMaterial->IsCompute() && Type != RenderType::RenderPass)
                        {
                            continue;
                        }

                        // Pass == 0 opaque materials
                        // Pass == 1 transparent materials
                        if ((MeshMaterial->IsTransparent() ? 1 : 0) != Pass)
                        {
                            continue;
                        }

                        BindRootSignatureOnce(!MeshMaterial->IsCompute());

                        // Update shared params
                        {
                            //  Update light handle
                            {
                                auto  Index           = MeshMaterial->IsTransparent() ? 0 : 1;
                                auto& LightDataHandle = MeshMaterial->IsTransparent() ? TransparentLightDataHandle : OpaqueLightDataHandle;
                                if (LastLightHandle[Index] != LightDataHandle)
                                {
                                    CommandList->SetDescriptorTable(
                                        !MeshMaterial->IsCompute(),
                                        uint32_t(RHI::RSCommon::MaterialRS::LightData),
                                        LastLightHandle[Index] = LightDataHandle);
                                }
                            }

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
                });

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