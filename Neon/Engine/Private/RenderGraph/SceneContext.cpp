#include <EnginePCH.hpp>
#include <RenderGraph/SceneContext.hpp>
#include <RenderGraph/Storage.hpp>
#include <Runtime/GameLogic.hpp>

#include <Scene/EntityWorld.hpp>
#include <Scene/GPU/Scene.hpp>
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
    }

    //

    void SceneContext::Render(
        RHI::ICommandList*       CommandList,
        RenderType               Type,
        RHI::GpuDescriptorHandle OpaqueLightDataHandle,
        RHI::GpuDescriptorHandle TransparentLightDataHandle) const
    {
        auto& GpuTransformManager = Runtime::GameLogic::Get()->GetGPUScene()->GetTransformManager();
        auto& GpuLightManager     = Runtime::GameLogic::Get()->GetGPUScene()->GetLightManager();

        auto& Meshes        = GpuTransformManager.GetMeshes();
        auto& MeshInstances = GpuTransformManager.GetMeshInstanceIds();

        // Accumulate count of instances
        size_t Count = 0;
        for (auto& InstanceList : MeshInstances)
        {
            Count += InstanceList.second.size();
        }

        if (!Count)
        {
            return;
        }

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

        RHI::GpuDescriptorHandle LastLightHandle[2];
        for (size_t Pass = 0; Pass < PassesCount; Pass++)
        {
#ifndef NEON_DIST
            if (Type != RenderType::DepthPrepass)
            {
                CommandList->BeginEvent(PassNames[Pass]);
            }
#endif

            auto PassType = Passes[Pass];
            for (auto& InstanceList : MeshInstances | std::views::values)
            {
                uint32_t FirstInstanceId = InstanceList.back();
                auto     FirstMesh       = Meshes.at(FirstInstanceId);
                auto&    FirstMaterial   = FirstMesh->GetModel()->GetMaterial(FirstMesh->GetData().MaterialIndex);

                bool WasSet              = false;
                auto UpdatePipelineState = [&]
                {
                    if (WasSet)
                    {
                        return;
                    }

                    WasSet = true;

                    CommandList->SetPipelineState(FirstMaterial->GetPipelineState(PassType));
                };

                if (FirstMaterial->IsCompute() && Type != RenderType::RenderPass)
                {
                    continue;
                }

                for (auto InstanceId : InstanceList)
                {
                    auto  Mesh         = Meshes.at(InstanceId);
                    auto& MeshData     = Mesh->GetData();
                    auto& MeshModel    = Mesh->GetModel();
                    auto& MeshMaterial = Mesh->GetModel()->GetMaterial(MeshData.MaterialIndex);

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
                    UpdatePipelineState();

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