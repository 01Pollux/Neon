#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Resource/Pack.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <Resource/Types/Shader.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Neon::Renderer
{
    namespace SpriteBatchConstants
    {
        constexpr uint32_t VerticesCount = 4;
        constexpr uint32_t IndicesCount  = 6;

        constexpr uint32_t MaxPoints   = 10'000;
        constexpr uint32_t MaxVertices = MaxPoints * VerticesCount;
        constexpr uint32_t MaxIndices  = MaxPoints * IndicesCount;

        using IndexType = uint16_t;
        static_assert(MaxIndices < std::numeric_limits<IndexType>::max());

        [[nodiscard]] Structured::CookedLayout CreateLayout()
        {
            Structured::RawLayout Layout;

            Layout.Append(Structured::Type::Float3, "Position");
            Layout.Append(Structured::Type::Float2, "TexCoord");
            Layout.Append(Structured::Type::Int, "TexIndex");
            Layout.Append(Structured::Type::Float4, "Color");

            return Layout.Cook(true);
        }

        /// <summary>
        /// a sprite's quad is made up of 4 vertices and looks like this:
        /// 0---1
        /// |   |
        /// 3---2
        /// </summary>
        void CreateIndexInstance(
            IndexType* IndexBuffer,
            IndexType& Offset)
        {
            IndexBuffer[Offset + 0] = Offset;
            IndexBuffer[Offset + 1] = Offset + 1;
            IndexBuffer[Offset + 2] = Offset + 2;

            IndexBuffer[Offset + 3] = Offset;
            IndexBuffer[Offset + 4] = Offset + 2;
            IndexBuffer[Offset + 5] = Offset + 3;

            Offset += IndicesCount;
        }
    } // namespace SpriteBatchConstants

    SpriteBatch::SpriteBatch(
        const CompiledShader& InitInfo,
        RHI::ISwapchain*      Swapchain) :
        SpriteBatch(CreatePipelineStates(InitInfo), Swapchain)
    {
    }

    SpriteBatch::SpriteBatch(
        const CompiledPipelineState& InitInfo,
        RHI::ISwapchain*             Swapchain) :
        m_BufferLayout(SpriteBatchConstants::CreateLayout())
    {
        CreateBuffers(Swapchain);
        CreatePipelineState(InitInfo);
    }

    //

    void SpriteBatch::Begin(
        RHI::IGraphicsCommandList* CommandList)
    {
        m_CommandList = CommandList;
        m_CommandList->SetRootSignature(m_RootSignature);

        m_ResourceView = m_CommandList->GetResourceView();
        m_SamplerView  = m_CommandList->GetSamplerView();

        m_DrawCount    = 0;
        m_TextureCount = 0;
    }

    void SpriteBatch::Draw(
        const QuadCommand& Quad)
    {
        if (m_DrawCount >= SpriteBatchConstants::MaxPoints)
        {
            auto CommandList = m_CommandList;
            End();
            Begin(CommandList);
        }

        if (Quad.Texture)
        {
            m_ResourceView.Bind(
                Quad.Texture.get(),
                nullptr,
                m_TextureCount++);
        }

        for (size_t i = 0; i < SpriteBatchConstants::VerticesCount; ++i)
        {
            auto Buffer        = m_BufferLayout.Access(m_VertexBufferPtr, m_DrawCount * SpriteBatchConstants::VerticesCount + i);
            Buffer["Position"] = Quad.Position;
            Buffer["TexCoord"] = Quad.TexCoord;
            Buffer["Color"]    = Quad.Color;

            if (Quad.Texture)
            {
                Buffer["TexIndex"] = m_TextureCount - 1;
            }
            else
            {
                Buffer["TexIndex"] = -1;
            }
        }
        m_DrawCount++;
    }

    void SpriteBatch::End()
    {
        auto& ResourceView = m_CommandList->GetResourceView();

        RHI::Views::Vertex VertexView;
        VertexView.Append(
            m_VertexBuffer->GetHandle(),
            m_BufferLayout.GetSize(),
            m_BufferLayout.GetSize() * SpriteBatchConstants::VerticesCount * m_DrawCount);

        RHI::Views::Index IndexView(
            m_IndexBuffer->GetHandle(),
            m_DrawCount * SpriteBatchConstants::IndicesCount);

        m_CommandList->SetVertexBuffer(0, VertexView);
        m_CommandList->SetIndexBuffer(IndexView);

        m_CommandList->Draw(
            RHI::DrawIndexArgs{
                .IndexCountPerInstance = m_DrawCount * SpriteBatchConstants::IndicesCount });

        m_CommandList = nullptr;
    }

    //

    auto SpriteBatch::CreatePipelineStates(
        const CompiledShader& InitInfo) -> CompiledPipelineState
    {
        InitInfo.Pack->LoadAsync(InitInfo.QuadRootSignature);
        InitInfo.Pack->LoadAsync(InitInfo.QuadVertexShader);
        InitInfo.Pack->LoadAsync(InitInfo.QuadPixelShader);

        auto RootSignature = InitInfo.Pack->Load<Asset::RootSignatureAsset>(InitInfo.QuadRootSignature)->GetRootSignature();

        RHI::PipelineStateBuilderG QuadPipelineState{
            .RootSignature  = RootSignature.get(),
            .VertexShader   = InitInfo.Pack->Load<Asset::ShaderAsset>(InitInfo.QuadVertexShader)->GetShader().get(),
            .PixelShader    = InitInfo.Pack->Load<Asset::ShaderAsset>(InitInfo.QuadPixelShader)->GetShader().get(),
            .Rasterizer     = { .CullMode = RHI::CullMode::None },
            .UseVertexInput = true
        };

        return {
            .QuadPipelineState = { RHI::IPipelineState::Create(QuadPipelineState) },
            .QuadRootSignature = RootSignature
        };
    }

    void SpriteBatch::CreateBuffers(
        RHI::ISwapchain* Swapchain)
    {
        m_VertexBuffer.reset(RHI::IUploadBuffer::Create(
            Swapchain,
            {
                .Size = m_BufferLayout.GetSize() * SpriteBatchConstants::MaxVertices,
            }));

        m_VertexBufferPtr = m_VertexBuffer->Map();

        m_IndexBuffer.reset(RHI::IUploadBuffer::Create(
            Swapchain,
            {
                .Size = sizeof(SpriteBatchConstants::IndexType) * SpriteBatchConstants::MaxIndices,
            }));

        auto IndexBuffer = m_IndexBuffer->Map<SpriteBatchConstants::IndexType>();

        SpriteBatchConstants::IndexType Offset = 0;
        while (Offset < SpriteBatchConstants::MaxIndices)
        {
            SpriteBatchConstants::CreateIndexInstance(IndexBuffer, Offset);
        }

        m_IndexBuffer->Unmap();
    }

    //

    void SpriteBatch::CreatePipelineState(
        const CompiledPipelineState& InitInfo)
    {
        m_PipelineState = InitInfo.QuadPipelineState;
        m_RootSignature = InitInfo.QuadRootSignature;
    }
} // namespace Neon::Renderer