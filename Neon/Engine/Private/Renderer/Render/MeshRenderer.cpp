#include <EnginePCH.hpp>
#include <Renderer/Render/MeshRenderer.hpp>
#include <Scene/EntityWorld.hpp>

#include <RHI/Resource/Views/Shader.hpp>
#include <RHI/GlobalBuffer.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::Renderer
{
    /// <summary>
    /// Update and apply the material to command list
    /// </summary>
    static void ApplyMeshMaterial(
        std::vector<RHI::IUploadBuffer*>& BuffersToUnmap,
        Renderer::IMaterial*              Material,
        RHI::GpuResourceHandle            CameraBuffer,
        const Component::Transform&       Transform,
        RHI::ICommandList*                CommandList);

    //

    MeshRenderer::MeshRenderer() :
        m_MeshQuery(
            Scene::EntityWorld::Get()
                .query_builder<Component::Transform, Component::MeshInstance>()
                .with<Component::ActiveSceneEntity>()
                .term<Component::Transform>()
                .in()
                .term<Component::MeshInstance>()
                .in()
                // Order by material's pipeline state
                .order_by<Component::MeshInstance>(
                    [](flecs::entity_t                EntLhs,
                       const Component::MeshInstance* MeshLhs,
                       flecs::entity_t                EntRhs,
                       const Component::MeshInstance* MeshRhs) -> int32_t
                    {
                        auto& ModelLhs = MeshLhs->Mesh.GetModel();
                        auto& ModelRhs = MeshRhs->Mesh.GetModel();

                        auto MatIdxLhs = MeshLhs->Mesh.GetSubmesh().MaterialIndex;
                        auto MatIdxRhs = MeshRhs->Mesh.GetSubmesh().MaterialIndex;

                        auto& MaterialLhs = ModelLhs->GetMaterial(MatIdxLhs);
                        auto& MaterialRhs = ModelRhs->GetMaterial(MatIdxRhs);

                        auto& PipelineStateLhs = MaterialLhs->GetPipelineState();
                        auto& PipelineStateRhs = MaterialRhs->GetPipelineState();

                        if (PipelineStateLhs < PipelineStateRhs)
                        {
                            return -1;
                        }
                        else if (PipelineStateLhs > PipelineStateRhs)
                        {
                            return 1;
                        }
                        return 0;
                    })
                .build())
    {
    }

    MeshRenderer::~MeshRenderer()
    {
        m_MeshQuery.destruct();
    }

    //

    void MeshRenderer::Render(
        RHI::GpuResourceHandle CameraBuffer,
        RHI::ICommandList*     CommandList)
    {
        if (m_MeshQuery.is_true())
        {
            std::vector<RHI::IUploadBuffer*> BuffersToUnmap;
            BuffersToUnmap.reserve(m_MeshQuery.count());

            CommandList->MarkEvent("Render Mesh");
            m_MeshQuery.iter(
                [this,
                 &BuffersToUnmap,
                 CameraBuffer,
                 CommandList](flecs::iter&                   Iter,
                              const Component::Transform*    Transform,
                              const Component::MeshInstance* Mesh)
                {
                    for (size_t i : Iter)
                    {
                        auto& CurTransform = Transform[i];
                        auto& CurMesh      = Mesh[i];

                        auto& Model   = CurMesh.Mesh.GetModel();
                        auto& Submesh = CurMesh.Mesh.GetSubmesh();

                        auto  MatIdx   = Submesh.MaterialIndex;
                        auto& Material = Model->GetMaterial(MatIdx);

                        ApplyMeshMaterial(
                            BuffersToUnmap,
                            Material.get(),
                            CameraBuffer,
                            CurTransform,
                            CommandList);

                        auto& VertexBuffer = Model->GetVertexBuffer();
                        auto& IndexBuffer  = Model->GetIndexBuffer();

                        RHI::Views::Vertex VertexView;
                        VertexView.AppendOffset<Mdl::MeshVertex>(
                            VertexBuffer->GetHandle(),
                            Submesh.VertexOffset,
                            Submesh.VertexCount);

                        RHI::Views::IndexU32 IndexView(
                            IndexBuffer.Get().get(),
                            Submesh.IndexOffset,
                            Submesh.IndexCount);

                        CommandList->SetPrimitiveTopology(
                            Submesh.Topology);

                        CommandList->SetVertexBuffer(0, VertexView);
                        CommandList->SetIndexBuffer(IndexView);

                        CommandList->Draw(
                            RHI::DrawIndexArgs{
                                .IndexCountPerInstance = Submesh.IndexCount });
                    }
                });

            for (auto Buffer : BuffersToUnmap)
            {
                Buffer->Unmap();
            }
        }
    }

    void ApplyMeshMaterial(
        std::vector<RHI::IUploadBuffer*>& BuffersToUnmap,
        Renderer::IMaterial*              Material,
        RHI::GpuResourceHandle            CameraBuffer,
        const Component::Transform&       Transform,
        RHI::ICommandList*                CommandList)
    {
        // Update per-frame data
        Material->SetResourceView(
            "g_FrameData",
            CameraBuffer);

        // Update per-object data
        {
            RHI::UBufferPoolHandle Buffer(
                sizeof(MeshRenderer::PerObjectData),
                1,
                RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);
            {
                auto UploadBuffer = Buffer.AsUpload();

                auto Data   = std::bit_cast<MeshRenderer::PerObjectData*>(UploadBuffer->Map() + Buffer.Offset);
                Data->World = Transform.World.ToMat4x4Transposed();

                BuffersToUnmap.push_back(UploadBuffer);
            }

            Material->SetResourceView(
                "v_ObjectData",
                Buffer.GetGpuHandle());
        }

        // Update per-material data
        {
            RHI::UBufferPoolHandle Buffer(
                sizeof(MeshRenderer::PerMaterialData),
                1,
                RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);
            {
                auto UploadBuffer = Buffer.AsUpload();

                auto Data = std::bit_cast<MeshRenderer::PerMaterialData*>(UploadBuffer->Map() + Buffer.Offset);
                // Temp, for now we won't enable any texture maps
                Data->Flags    = 0;
                Data->Albedo   = Colors::White;
                Data->Specular = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
                Data->Emissive = Colors::Black;

                BuffersToUnmap.push_back(UploadBuffer);
            }

            Material->SetResourceView(
                "p_MaterialData",
                Buffer.GetGpuHandle());
        }

        // Finalize material
        Material->Apply(CommandList);
    }
} // namespace Neon::Renderer