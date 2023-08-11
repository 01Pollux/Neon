#include <EnginePCH.hpp>

#ifndef NEON_DIST

#include <Runtime/DebugOverlay.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

#include <Renderer/Material/Builder.hpp>
#include <Renderer/Material/Material.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/Commands/List.hpp>
#include <RHI/Resource/Resource.hpp>
#include <RHI/Resource/Views/Shader.hpp>

#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    namespace AssetGuids
    {
        auto DebugOverlayGuid()
        {
            return Asset::Handle::FromString("bbd67e6b-afc3-4d55-bbda-ed382b3342f5");
        }
    } // namespace AssetGuids

    class DefaultEngineDebugOverlay : public DebugOverlay
    {
        struct Overlay_Debug_Line
        {
            struct Vertex
            {
                Vector3  Position;
                Color4U8 Color;
                int      NeedsProjection;
            };

            static constexpr uint32_t VerticesCount = 2;
            static constexpr uint32_t MaxPoints     = 10'000;
            static constexpr uint32_t MaxVertices   = MaxPoints * VerticesCount;
        };

        struct Overlay_Debug_LineBuffer
        {
            Ptr<Renderer::IMaterial> Material;

            std::vector<UPtr<RHI::IUploadBuffer>> VertexBuffers;

            Overlay_Debug_Line::Vertex* VertexBufferPtr = nullptr;
            uint32_t                    DrawCount       = 0;

            Overlay_Debug_LineBuffer();

            bool ShouldDraw() const;

            void Flush(
                RHI::IGraphicsCommandList* CommandList,
                RHI::GpuResourceHandle     PerFrameData);

            void Append(
                const Vector3& StartPosition,
                const Vector3& EndPosition,
                const Color4&  StartColor,
                const Color4&  EndColor,
                bool           WorldsSpace);

            void Reset();
        };

    public:
        bool ShouldRender_Impl() override;

        void Reset_Impl() override;

        void Render_Impl(
            RHI::IGraphicsCommandList* CommandList,
            RHI::GpuResourceHandle     PerFrameData) override;

        void DrawLine_Impl(
            const Vector3& StartPosition,
            const Vector3& EndPosition,
            const Color4&  StartColor,
            const Color4&  EndColor,
            bool           WorldsSpace) override;

        void DrawCuboidLine_Impl(
            const Vector3& CenterPosition,
            const Vector3& Size,
            const Color4&  Color,
            bool           WorldsSpace) override;

    private:
        Overlay_Debug_LineBuffer m_LineBuffer;
    };

    void DebugOverlay::Create()
    {
        NEON_ASSERT(!s_DebugOverlay, "Debug overlay already created");
        s_DebugOverlay = new DefaultEngineDebugOverlay;
    }

    void DebugOverlay::Destroy()
    {
        NEON_ASSERT(s_DebugOverlay, "Debug overlay not created");
        delete s_DebugOverlay;
        s_DebugOverlay = nullptr;
    }

    //

    bool DefaultEngineDebugOverlay::ShouldRender_Impl()
    {
        return m_LineBuffer.ShouldDraw();
    }

    void DefaultEngineDebugOverlay::Reset_Impl()
    {
        m_LineBuffer.Reset();
    }

    void DefaultEngineDebugOverlay::Render_Impl(
        RHI::IGraphicsCommandList* CommandList,
        RHI::GpuResourceHandle     PerFrameData)
    {
        m_LineBuffer.Flush(CommandList, PerFrameData);
    }

    void DefaultEngineDebugOverlay::DrawLine_Impl(
        const Vector3& StartPosition,
        const Vector3& EndPosition,
        const Color4&  StartColor,
        const Color4&  EndColor,
        bool           WorldsSpace)
    {
        m_LineBuffer.Append(StartPosition, EndPosition, StartColor, EndColor, WorldsSpace);
    }

    void DefaultEngineDebugOverlay::DrawCuboidLine_Impl(
        const Vector3& CenterPosition,
        const Vector3& Size,
        const Color4&  Color,
        bool           WorldsSpace)
    {
        const Vector3 HalfSize = Size / 2.f;

        /*
        X: center
        0---1      4---5
        |   |  X   |   |
        3---2      7---6
        <----------> == half size
        */
        const Vector3 Positions[]{
            { CenterPosition.x - HalfSize.x, CenterPosition.y + HalfSize.y, CenterPosition.z - HalfSize.z }, // 0 -- back -- top left
            { CenterPosition.x + HalfSize.x, CenterPosition.y + HalfSize.y, CenterPosition.z - HalfSize.z }, // 1 -- back -- top right
            { CenterPosition.x + HalfSize.x, CenterPosition.y - HalfSize.y, CenterPosition.z - HalfSize.z }, // 2 -- back -- bottom right
            { CenterPosition.x - HalfSize.x, CenterPosition.y - HalfSize.y, CenterPosition.z - HalfSize.z }, // 3 -- back -- bottom left
            { CenterPosition.x - HalfSize.x, CenterPosition.y + HalfSize.y, CenterPosition.z + HalfSize.z }, // 4 -- front -- top left
            { CenterPosition.x + HalfSize.x, CenterPosition.y + HalfSize.y, CenterPosition.z + HalfSize.z }, // 5 -- front -- top right
            { CenterPosition.x + HalfSize.x, CenterPosition.y - HalfSize.y, CenterPosition.z + HalfSize.z }, // 6 -- front -- bottom right
            { CenterPosition.x - HalfSize.x, CenterPosition.y - HalfSize.y, CenterPosition.z + HalfSize.z }  // 7 -- front -- bottom left
        };

        constexpr uint8_t Indices[]{
            0, 1,
            1, 2,
            2, 3,
            3, 0,

            4, 5,
            5, 6,
            6, 7,
            7, 4,

            0, 4,
            1, 5,
            2, 6,
            3, 7
        };

        for (size_t i = 0; i < std::size(Indices); i += 2)
        {
            const Vector3& P1 = Positions[Indices[i]];
            const Vector3& P2 = Positions[Indices[i + 1]];

            m_LineBuffer.Append(P1, P2, Color, Color, WorldsSpace);
        }
    }

    //
    //
    //

    // Line
    DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::Overlay_Debug_LineBuffer()
    {
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr DebugShader = Asset::Manager::Load(AssetGuids::DebugOverlayGuid());

        RHI::ShaderInputLayout VertexInput;
        VertexInput.emplace_back("Position", RHI::EResourceFormat::R32G32B32_Float);
        VertexInput.emplace_back("Color", RHI::EResourceFormat::R8G8B8A8_UNorm);
        VertexInput.emplace_back("NeedsProjection", RHI::EResourceFormat::R32_UInt);

        Material =
            Renderer::RenderMaterialBuilder()
                .RootSignature(
                    RHI::IRootSignature::Create(
                        RHI::RootSignatureBuilder()
                            .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout)
                            .AddConstantBufferView("g_FrameData", 0, 1)))
                .Rasterizer(Renderer::MaterialStates::Rasterizer::CullNone)
                .DepthStencil(Renderer::MaterialStates::DepthStencil::None)
                .VertexShader(DebugShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex }))
                .InputLayout(std::move(VertexInput))
                .PixelShader(DebugShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel }))
                .Topology(RHI::PrimitiveTopologyCategory::Line)
                .RenderTarget(0, RHI::ISwapchain::Get()->GetFormat())
                .Build();

        Asset::Manager::Unload(DebugShader->GetGuid());
    }

    bool DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::ShouldDraw() const
    {
        return !VertexBuffers.empty();
    }

    void DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::Flush(
        RHI::IGraphicsCommandList* CommandList,
        RHI::GpuResourceHandle     PerFrameData)
    {
        if (!ShouldDraw())
        {
            return;
        }

        VertexBuffers.back()->Unmap();

        CommandList->SetRootSignature(Material->GetRootSignature());
        CommandList->SetResourceView(RHI::IGraphicsCommandList::ViewType::Cbv, 0, PerFrameData);

        CommandList->SetPipelineState(Material->GetPipelineState());
        CommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::LineList);

        RHI::Views::Vertex VtxView;
        for (auto& VertexBuffer : VertexBuffers)
        {
            VtxView.Append(VertexBuffer.get(), 0, sizeof(Overlay_Debug_Line::Vertex), VertexBuffer->GetSize());
        }

        CommandList->SetVertexBuffer(0, VtxView);
        CommandList->Draw(RHI::DrawArgs{ .VertexCountPerInstance = DrawCount });
    }

    void DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::Append(
        const Vector3& StartPosition,
        const Vector3& EndPosition,
        const Color4&  StartColor,
        const Color4&  EndColor,
        bool           WorldsSpace)
    {
        if (DrawCount >= Overlay_Debug_Line::MaxVertices || VertexBuffers.empty()) [[unlikely]]
        {
            if (!VertexBuffers.empty())
            {
                VertexBuffers.back()->Unmap();
            }
            VertexBuffers.emplace_back(RHI::IUploadBuffer::Create({ .Size = Overlay_Debug_Line::MaxVertices * sizeof(Overlay_Debug_Line::Vertex) }));
            VertexBufferPtr = VertexBuffers.back()->Map<Overlay_Debug_Line::Vertex>();
        }

        auto CurVertex = VertexBufferPtr + DrawCount;
        DrawCount += Overlay_Debug_Line::VerticesCount;

        CurVertex[0].Position        = StartPosition;
        CurVertex[0].Color           = (StartColor * 255.f);
        CurVertex[0].NeedsProjection = WorldsSpace;

        CurVertex[1].Position        = EndPosition;
        CurVertex[1].Color           = (EndColor * 255.f);
        CurVertex[1].NeedsProjection = WorldsSpace;
    }

    void DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::Reset()
    {
        DrawCount = 0;
        VertexBuffers.clear();
    }
} // namespace Neon::Runtime

#endif // NEON_DIST