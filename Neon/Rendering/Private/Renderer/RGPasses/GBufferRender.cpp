#include <EnginePCH.hpp>
#include <Renderer/RGPasses/GBuffer.hpp>

#include <Runtime/GameLogic.hpp>

#include <Resource/Handler/MaterialResource.hpp>

#include <DirectXColors.h>
#include <Renderer/Material/UnlitMaterial.hpp>

namespace Renderer::RG
{
    void GBufferPass::Execute(
        const RenderGraphStorage& GraphStorage,
        RHI::ICommandContext&     CmdContext)
    {
        if (!m_Material.GetPipelineState().Get())
        {
            m_Material = UnlitMaterial();
        }

        auto& GraphicsContext = dynamic_cast<RHI::GraphicsCommandContext&>(CmdContext);

        auto& RootSig       = m_Material.GetRootSignature();
        auto& PipelineState = m_Material.GetPipelineState();

        GraphicsContext.SetRootSignature(RootSig);
        GraphicsContext.SetPipelineState(PipelineState);

        {
            using namespace DX;

            struct Vertex
            {
                XMFLOAT4 Color;
                XMFLOAT3 Pos;
            };

            std::array vertices = {
                Vertex({ XMFLOAT4(DX::Colors::Crimson), XMFLOAT3(-1.0f, -1.0f, -1.0f) }),
                Vertex({ XMFLOAT4(DX::Colors::Orange), XMFLOAT3(-1.0f, +1.0f, -1.0f) }),
                Vertex({ XMFLOAT4(DX::Colors::Yellow), XMFLOAT3(+1.0f, +1.0f, -1.0f) }),
                Vertex({ XMFLOAT4(DX::Colors::Orange), XMFLOAT3(+1.0f, -1.0f, -1.0f) }),
                Vertex({ XMFLOAT4(DX::Colors::Crimson), XMFLOAT3(-1.0f, -1.0f, +1.0f) }),
                Vertex({ XMFLOAT4(DX::Colors::Orange), XMFLOAT3(-1.0f, +1.0f, +1.0f) }),
                Vertex({ XMFLOAT4(DX::Colors::Yellow), XMFLOAT3(+1.0f, +1.0f, +1.0f) }),
                Vertex({ XMFLOAT4(DX::Colors::Orange), XMFLOAT3(+1.0f, -1.0f, +1.0f) })
            };

            std::array<std::uint16_t, 36> indices = {
                // front face
                0, 1, 2,
                0, 2, 3,

                // back face
                4, 6, 5,
                4, 7, 6,

                // left face
                4, 5, 1,
                4, 1, 0,

                // right face
                3, 2, 6,
                3, 6, 7,

                // top face
                1, 5, 6,
                1, 6, 2,

                // bottom face
                4, 0, 3,
                4, 3, 7
            };

            const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
            const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

            auto VtxBuffer = GraphicsContext.ReserveBuffer<RHI::GraphicsUploadBuffer>(vbByteSize);
            auto IdxBuffer = GraphicsContext.ReserveBuffer<RHI::GraphicsUploadBuffer>(ibByteSize);

            VtxBuffer->Write(0, vertices.data(), VtxBuffer->GetSize());
            IdxBuffer->Write(0, indices.data(), IdxBuffer->GetSize());

            //

            D3D12_VERTEX_BUFFER_VIEW VtxView{
                .BufferLocation = VtxBuffer->GetGpuAddress(),
                .SizeInBytes    = uint32_t(VtxBuffer->GetSize()),
                .StrideInBytes  = sizeof(Vertex)
            };
            GraphicsContext.SetVertexBuffers(
                &VtxView,
                1,
                0);

            GraphicsContext.SetIndexBuffer(
                { .BufferLocation = IdxBuffer->GetGpuAddress(),
                  .SizeInBytes    = uint32_t(IdxBuffer->GetSize()),
                  .Format         = DXGI_FORMAT_R16_UINT });
        }

        auto TestBuffer = GraphicsContext.ReserveBuffer<RHI::GraphicsUploadBuffer>(256, 256);

        struct TestStruct
        {
            float a;
            float b;
            float c;
            float d;
        };
        TestBuffer->WriteConstruct<TestStruct>(0, 1.0f, 2.0f, 3.0f, 4.0f);

        m_Material.Set("TestBuffer", TestBuffer);
        m_Material.Set("TestBuffer", TestBuffer, 2);

        TestBuffer = GraphicsContext.ReserveBuffer<RHI::GraphicsUploadBuffer>(256, 256);

        TestBuffer->WriteConstruct<TestStruct>(0, 21.0f, 32.0f, 3.0f, 4.0f);

        m_Material.Set("TestBuffer", TestBuffer, 1);

        m_Material.Update(GraphicsContext);

        GraphicsContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        GraphicsContext.Draw(RHI::DrawIndexArgs{
            .IndexCountPerInstance = 36 });

        RHI::GraphicsResource::Ptr Res;
        RHI::DescriptorViewDesc    Desc;
        m_Material.Get("TestBuffer", &Res, &Desc);
        m_Material.Get("TestBuffer", &Res, &Desc, 1);
        m_Material.Get("TestBuffer", &Res, &Desc, 2);
    }
} // namespace Renderer::RG
