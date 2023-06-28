#pragma once

#include <RHI/Resource/Resource.hpp>
#include <Utils/Struct.hpp>

#include <Resource/Pack.hpp>
#include <Resource/Handle.hpp>

#include <Math/Vector.hpp>
#include <Math/Rect.hpp>
#include <Math/Colors.hpp>

namespace Neon
{
    namespace RHI
    {
        class ISwapchain;
        class IPipelineState;
    } // namespace RHI
} // namespace Neon

namespace Neon::Renderer
{
    class SpriteBatch
    {
    public:
        struct QuadCommand
        {
            Vector2 Position;
            Vector2 Size;
            float   Rotation = 0.f;
            float   Depth    = 0.f;
            Color4  Color    = Colors::White;

            Ptr<RHI::ITexture> Texture;
            RectF              TexCoord = RectF(Vec::Zero<Vector2>, Vec::One<Vector2>);
        };

    public:
        struct CompiledShader
        {
            Asset::IAssetPack* Pack;

            Asset::AssetHandle QuadVertexShader;
            Asset::AssetHandle QuadPixelShader;
            Asset::AssetHandle QuadRootSignature;
        };

        struct CompiledPipelineState
        {
            Ptr<RHI::IPipelineState> QuadPipelineState;
        };

        SpriteBatch(
            const CompiledShader& InitInfo,
            RHI::ISwapchain*      Swapchain);

        SpriteBatch(
            const CompiledPipelineState& InitInfo,
            RHI::ISwapchain*             Swapchain);

        /// <summary>
        /// Begins drawing.
        /// </summary>
        void Begin();

        /// <summary>
        /// Enqueues a quad to be drawn.
        /// </summary>
        void Draw(
            const QuadCommand& Quad);

        /// <summary>
        /// Ends drawing.
        /// </summary>
        void End();

    private:
        /// <summary>
        /// Creates the pipeline states.
        /// </summary>
        static CompiledPipelineState CreatePipelineStates(
            const CompiledShader& InitInfo);

        /// <summary>
        /// Creates the vertex and index buffers.
        /// </summary>
        void CreateBuffers(
            RHI::ISwapchain* Swapchain);

        /// <summary>
        /// Creates the pipeline state.
        /// </summary>
        void CreatePipelineState(
            const CompiledPipelineState& InitInfo);

    private:
        Structured::CookedLayout m_BufferLayout;

        Ptr<RHI::IPipelineState> m_PipelineState;

        UPtr<RHI::IUploadBuffer> m_VertexBuffers;
        UPtr<RHI::IUploadBuffer> m_IndexBuffer;

        uint8_t* m_VertexBufferPtr = nullptr;

        size_t m_DrawCount = 0;
    };
} // namespace Neon::Renderer