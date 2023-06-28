#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Resource/Pack.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <Resource/Types/Shader.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Resource/State.hpp>

namespace Neon::Renderer
{
    namespace SpriteBatchConstants
    {
        constexpr size_t MaxPoints   = 10'000;
        constexpr size_t MaxVertices = MaxPoints * 4;
        constexpr size_t MaxIndices  = MaxPoints * 6;

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

            Offset += 6;
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

    void SpriteBatch::Begin()
    {
    }

    void SpriteBatch::Draw(
        const QuadCommand& Quad)
    {
    }

    void SpriteBatch::End()
    {
    }

    //

    auto SpriteBatch::CreatePipelineStates(
        const CompiledShader& InitInfo) -> CompiledPipelineState
    {
        InitInfo.Pack->LoadAsync(InitInfo.QuadRootSignature);
        InitInfo.Pack->LoadAsync(InitInfo.QuadVertexShader);
        InitInfo.Pack->LoadAsync(InitInfo.QuadPixelShader);

        RHI::PipelineStateBuilderG QuadPipelineState{
            .RootSignature  = InitInfo.Pack->Load<Asset::RootSignatureAsset>(InitInfo.QuadRootSignature)->GetRootSignature().get(),
            .VertexShader   = InitInfo.Pack->Load<Asset::ShaderAsset>(InitInfo.QuadVertexShader)->GetShader().get(),
            .PixelShader    = InitInfo.Pack->Load<Asset::ShaderAsset>(InitInfo.QuadPixelShader)->GetShader().get(),
            .Rasterizer     = { .CullMode = RHI::CullMode::None },
            .UseVertexInput = true
        };

        return {
            .QuadPipelineState = { RHI::IPipelineState::Create(QuadPipelineState) }
        };
    }

    void SpriteBatch::CreateBuffers(
        RHI::ISwapchain* Swapchain)
    {
        m_VertexBuffers.reset(RHI::IUploadBuffer::Create(
            Swapchain,
            {
                .Size = m_BufferLayout.GetSize() * SpriteBatchConstants::MaxVertices,
            }));

        m_VertexBufferPtr = m_VertexBuffers->Map();

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
    }
} // namespace Neon::Renderer