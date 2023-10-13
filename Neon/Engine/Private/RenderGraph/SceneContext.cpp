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

        constexpr size_t       SizeOfBlock = SizeOfPerMaterialData + Scene::GPUTransformManager::SizeOfInstanceData;
        RHI::UBufferPoolHandle Buffer(
            SizeOfBlock * Count,
            std::max(AlignOfPerMaterialData, Scene::GPUTransformManager::AlignOfInstanceData),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        uint8_t* BufferPtr     = Buffer.AsUpload().Map() + Buffer.Offset;
        size_t   InstanceIndex = 0;
        size_t   PassesCount   = 1;

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
                    FirstMaterial->SetResourceView(
                        "_FrameConstant",
                        m_Storage.GetFrameDataHandle());

                    CommandList->SetRootSignature(!FirstMaterial->IsCompute(), FirstMaterial->GetRootSignature());
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

                    UpdatePipelineState();

                    // Update shared params
                    {
                        auto MaterialData    = std::bit_cast<PerMaterialData*>(BufferPtr);
                        *MaterialData        = {};
                        MaterialData->Albedo = Colors::DarkGreen;

                        auto InstanceData = std::bit_cast<Scene::GPUTransformManager::InstanceData*>(BufferPtr + SizeOfPerMaterialData);
                        *InstanceData     = *GpuTransformManager.GetInstanceData(InstanceId);

                        MeshMaterial->SetResourceView("_PerMaterialData", Buffer.GetGpuHandle(SizeOfBlock * InstanceIndex));
                        MeshMaterial->SetResourceView("_PerInstanceData", Buffer.GetGpuHandle(SizeOfBlock * InstanceIndex + SizeOfPerMaterialData));

                        MeshMaterial->BindSharedParams(CommandList);
                        {
                            auto& RootSignature = MeshMaterial->GetRootSignature();
                            if (auto LightingParam = RootSignature->FindParamIndex("_LightData"))
                            {
                                auto& LightDataHandle = MeshMaterial->IsTransparent() ? TransparentLightDataHandle : OpaqueLightDataHandle;
                                if (LightDataHandle)
                                {
                                    CommandList->SetDescriptorTable(
                                        !MeshMaterial->IsCompute(),
                                        LightingParam,
                                        LightDataHandle);
                                }
                            }
                        }
                    }

                    MeshMaterial->BindLocalParams(CommandList);

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

                    BufferPtr += SizeOfBlock;
                    InstanceIndex++;
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