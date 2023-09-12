#include <EnginePCH.hpp>
#include <Renderer/Render/MeshRenderer.hpp>
#include <Scene/EntityWorld.hpp>

#include <RHI/Resource/Views/Shader.hpp>
#include <RHI/GlobalBuffer.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::Renderer
{
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

    struct PerObjectData
    {
        Matrix4x4 World;
    };

    void MeshRenderer::Render(
        RHI::GpuResourceHandle CameraBuffer,
        RHI::ICommandList*     CommandList)
    {
        if (m_MeshQuery.is_true())
        {
            m_MeshQuery.iter(
                [this,
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

                        Material->SetResourceView(
                            "g_FrameData",
                            CameraBuffer);

                        RHI::UBufferPoolHandle Buffer(
                            Math::AlignUp(sizeof(PerObjectData), RHI::ConstantBufferAlignement),
                            RHI::ConstantBufferAlignement,
                            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);
                        {
                            auto Data   = std::bit_cast<PerObjectData*>(Buffer.AsUpload()->Map() + Buffer.Offset);
                            Data->World = CurTransform.World.ToMat4x4Transposed();
                            Buffer.AsUpload()->Unmap();
                        }

                        Material->SetResourceView(
                            "v_PerObjectData",
                            Buffer.GetGpuHandle());

                        Material->Apply(CommandList);

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
        }
    }
} // namespace Neon::Renderer