#include <EnginePCH.hpp>
#include <Renderer/Render/MeshRenderer.hpp>
#include <Scene/EntityWorld.hpp>

#include <RHI/Resource/Views/Shader.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>

#include <RHI/GlobalDescriptors.hpp>
#include <RHI/GlobalBuffer.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::Renderer
{
    static constexpr size_t SizeOf_PerMaterialData = Math::AlignUp(sizeof(MeshRenderer::PerMaterialData), RHI::ShaderResourceAlignement);
    static constexpr size_t SizeOf_PerObjectData   = Math::AlignUp(sizeof(MeshRenderer::PerObjectData), RHI::ConstantBufferAlignement);

    /// <summary>
    /// Update and apply the material to command list
    /// </summary>
    static void ApplyMeshMaterial(
        MeshRenderer::FrameResource& FrameResource,
        size_t                       SubresourceIndex,
        Renderer::IMaterial*         Material,
        RHI::GpuResourceHandle       CameraBuffer,
        const Component::Transform&  Transform,
        RHI::ICommandList*           CommandList);

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
            size_t SubresourceIndex = 0;
            CommandList->MarkEvent("Render Mesh");
            m_MeshQuery.iter(
                [&](flecs::iter&                   Iter,
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
                            m_MeshData.Get(),
                            SubresourceIndex++,
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
        }
    }

    void ApplyMeshMaterial(
        MeshRenderer::FrameResource& FrameResource,
        size_t                       SubresourceIndex,
        Renderer::IMaterial*         Material,
        RHI::GpuResourceHandle       CameraBuffer,
        const Component::Transform&  Transform,
        RHI::ICommandList*           CommandList)
    {
        CommandList->SetRootSignature(true, Material->GetRootSignature());
        CommandList->SetPipelineState(Material->GetPipelineState());

        CommandList->SetResourceView(true, RHI::CstResourceViewType::Cbv, 1, CameraBuffer);
        // Update per-frame data
        /*     Material->SetResourceView(
                 "g_FrameData",
                 CameraBuffer);*/

        struct
        {
            uint32_t PerObjectOffset = 0;
        } CBInfo;

        auto Descriptor        = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(2);
        CBInfo.PerObjectOffset = Descriptor.Offset;

        // Update per-material data
        {
            size_t Offset = SizeOf_PerMaterialData * SubresourceIndex;

            auto& Buffer = FrameResource.PerMaterialBuffer;
            {
                auto Data = std::bit_cast<MeshRenderer::PerMaterialData*>(Buffer->Map() + Offset);
                // Temp, for now we won't enable any texture maps
                Data->Flags    = 0;
                Data->Albedo   = Colors::White;
                Data->Specular = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
                Data->Emissive = Colors::Black;
            }

            RHI::SRVDesc SrvDesc{
                .View = RHI::SRVDesc::Buffer{
                    .FirstElement = SubresourceIndex,
                    .Count        = 1,
                    .SizeOfStruct = SizeOf_PerMaterialData }
            };

            Descriptor->CreateShaderResourceView(
                Descriptor.Offset + 1,
                Buffer.get(),
                &SrvDesc);

            /*Material->SetResourceView(
                "p_MaterialData",
                Buffer->GetHandle(Offset));*/
        }

        // Update per-object data
        {
            size_t Offset = SizeOf_PerObjectData * SubresourceIndex;

            auto& Buffer = FrameResource.PerObjectBuffer;
            {
                auto Data   = std::bit_cast<MeshRenderer::PerObjectData*>(Buffer->Map() + Offset);
                Data->World = Transform.World.ToMat4x4Transposed();
            }

            RHI::SRVDesc SrvDesc{
                .View = RHI::SRVDesc::Buffer{
                    .FirstElement = SubresourceIndex,
                    .Count        = 1,
                    .SizeOfStruct = SizeOf_PerObjectData }
            };

            Descriptor->CreateShaderResourceView(
                Descriptor.Offset,
                Buffer.get(),
                &SrvDesc);

            /*Material->SetResourceView(
                "v_ObjectData",
                Buffer->GetHandle(Offset));*/
        }

        CommandList->SetDynamicResourceView(
            true,
            RHI::CstResourceViewType::Cbv,
            0,
            &CBInfo,
            sizeof(CBInfo));

        for (uint32_t i = 2; i <= 55; i++)
        {
            CommandList->SetDescriptorTable(
                true,
                i,
                Descriptor->GetGPUAddress());
        }

        // Finalize material
        // Material->Apply(CommandList);
    }

    MeshRenderer::FrameResource::FrameResource() :
        PerObjectBuffer(
            RHI::IGpuResource::Create(
                RHI::ResourceDesc::BufferUpload(SizeOf_PerObjectData * 1024))),
        PerMaterialBuffer(
            RHI::IGpuResource::Create(
                RHI::ResourceDesc::BufferUpload(SizeOf_PerMaterialData * 1024)))
    {
    }
} // namespace Neon::Renderer