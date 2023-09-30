#include <EnginePCH.hpp>
#include <RenderGraph/SceneContext.hpp>
#include <RenderGraph/Storage.hpp>

#include <Runtime/GameLogic.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/GPUScene.hpp>

#include <Mdl/Mesh.hpp>
#include <RHI/Material/Material.hpp>
#include <RHI/Resource/Views/Shader.hpp>
#include <RHI/GlobalBuffer.hpp>

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
        RHI::ICommandList* CommandList,
        RenderType         Type) const
    {
        auto  GpuScene      = Runtime::GameLogic::Get()->GetGPUScene();
        auto& Meshes        = GpuScene->GetMeshes();
        auto& MeshInstances = GpuScene->GetMeshInstanceIds();

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

        RHI::UBufferPoolHandle Buffer(
            SizeOfPerMaterialData * Count,
            AlignOfPerMaterialData,
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        auto   MaterialData  = Buffer.AsUpload().Map<PerMaterialData>(Buffer.Offset);
        size_t MaterialIndex = 0;

        for (auto& InstanceList : MeshInstances | std::views::values)
        {
            uint32_t FirstInstanceId = InstanceList.back();
            auto     FirstMesh       = Meshes.at(FirstInstanceId);
            auto&    FirstMaterial   = FirstMesh->GetModel()->GetMaterial(FirstMesh->GetData().MaterialIndex);

            if (FirstMaterial->IsCompute() && Type != RenderType::RenderPass)
            {
                continue;
            }

            Ptr<RHI::IPipelineState> PipelineState;
            switch (Type)
            {
            case RenderType::DepthPrepass:
                PipelineState = FirstMaterial->GetPipelineState(RHI::IMaterial::PipelineVariant::DepthPrePass);
                break;
            case RenderType::RenderPass:
                PipelineState = FirstMaterial->GetPipelineState(RHI::IMaterial::PipelineVariant::RenderPass);
                break;
            }

            FirstMaterial->SetResourceView(
                "_FrameConstant",
                m_Storage.GetFrameDataHandle());

            CommandList->SetRootSignature(!FirstMaterial->IsCompute(), FirstMaterial->GetRootSignature());
            CommandList->SetPipelineState(PipelineState);
            // FirstMaterial->BindSharedParams(CommandList);

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

                // Update shared params
                {
                    *MaterialData        = {};
                    MaterialData->Albedo = Colors::DarkGreen;

                    MeshMaterial->SetResourceView("_PerInstanceData", GpuScene->GetInstanceHandle(InstanceId));
                    MeshMaterial->SetResourceView("_PerMaterialData", Buffer.GetGpuHandle(SizeOfPerMaterialData * MaterialIndex));

                    MeshMaterial->BindSharedParams(CommandList);
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

                MaterialData++;
                MaterialIndex++;
            }
        }
    }
} // namespace Neon::RG