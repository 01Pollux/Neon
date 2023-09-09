#include <EnginePCH.hpp>

#ifndef NEON_DIST

#include <Runtime/GameEngine.hpp>
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

            std::multimap<float, std::pair<LineArgs, bool>> m_TimedLines;
            std::vector<UPtr<RHI::IUploadBuffer>>           VertexBuffers;

            Overlay_Debug_Line::Vertex* VertexBufferPtr = nullptr;
            uint32_t                    DrawCount       = 0;

            Overlay_Debug_LineBuffer();

            bool ShouldDraw() const;

            void Flush(
                RHI::IGraphicsCommandList* CommandList,
                RHI::GpuResourceHandle     PerFrameData);

            void Append(
                const LineArgs& Args,
                bool            WorldsSpace);

            void Append(
                float           Duration,
                const LineArgs& Args,
                bool            WorldsSpace);

            void Reset();
        };

    public:
        bool ShouldRender_Impl() override;

        void Reset_Impl() override;

        void Render_Impl(
            RHI::IGraphicsCommandList* CommandList,
            RHI::GpuResourceHandle     PerFrameData) override;

        void DrawLine_Impl(
            const LineArgs& Args,
            bool            WorldsSpace) override;

        void DrawLine_Impl(
            float           Duration,
            const LineArgs& Args,
            bool            WorldsSpace) override;

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

    void DebugOverlay::DrawCuboidLine_Impl(
        float           Duration,
        const CubeArgs& Args,
        bool            WorldsSpace)
    {
        const Vector3 HalfSize = Args.Size / 2.f;

        /*
        X: center
        0---1      4---5
        |   |  X   |   |
        3---2      7---6
        <----------> == half size
        */
        const Vector3 Positions[]{
            { Args.CenterPosition.x - HalfSize.x, Args.CenterPosition.y + HalfSize.y, Args.CenterPosition.z - HalfSize.z }, // 0 -- back -- top left
            { Args.CenterPosition.x + HalfSize.x, Args.CenterPosition.y + HalfSize.y, Args.CenterPosition.z - HalfSize.z }, // 1 -- back -- top right
            { Args.CenterPosition.x + HalfSize.x, Args.CenterPosition.y - HalfSize.y, Args.CenterPosition.z - HalfSize.z }, // 2 -- back -- bottom right
            { Args.CenterPosition.x - HalfSize.x, Args.CenterPosition.y - HalfSize.y, Args.CenterPosition.z - HalfSize.z }, // 3 -- back -- bottom left
            { Args.CenterPosition.x - HalfSize.x, Args.CenterPosition.y + HalfSize.y, Args.CenterPosition.z + HalfSize.z }, // 4 -- front -- top left
            { Args.CenterPosition.x + HalfSize.x, Args.CenterPosition.y + HalfSize.y, Args.CenterPosition.z + HalfSize.z }, // 5 -- front -- top right
            { Args.CenterPosition.x + HalfSize.x, Args.CenterPosition.y - HalfSize.y, Args.CenterPosition.z + HalfSize.z }, // 6 -- front -- bottom right
            { Args.CenterPosition.x - HalfSize.x, Args.CenterPosition.y - HalfSize.y, Args.CenterPosition.z + HalfSize.z }  // 7 -- front -- bottom left
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

            if (Duration < 0.f)
                DrawLine_Impl({ P1, Args.Color, P2, Args.Color }, WorldsSpace);
            else
                DrawLine_Impl(Duration, { P1, Args.Color, P2, Args.Color }, WorldsSpace);
        }
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
        const LineArgs& Args,
        bool            WorldsSpace)
    {
        m_LineBuffer.Append(Args, WorldsSpace);
    }

    void DefaultEngineDebugOverlay::DrawLine_Impl(
        float           Duration,
        const LineArgs& Args,
        bool            WorldsSpace)
    {
        m_LineBuffer.Append(Duration, Args, WorldsSpace);
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
        return !VertexBuffers.empty() || !m_TimedLines.empty();
    }

    void DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::Flush(
        RHI::IGraphicsCommandList* CommandList,
        RHI::GpuResourceHandle     PerFrameData)
    {
        // Remove any expired lines from the timed line buffer
        if (!m_TimedLines.empty())
        {
            float GameTime = float(GameEngine::Get()->GetEngineTime());
            auto  Iter     = m_TimedLines.upper_bound(GameTime);
            if (Iter != m_TimedLines.begin())
            {
                m_TimedLines.erase(m_TimedLines.begin(), Iter);
            }
            for (auto& [Args, WorldsSpace] : m_TimedLines |
                                                 std::views::values)
            {
                Append(Args, WorldsSpace);
            }
        }

        if (!ShouldDraw())
        {
            return;
        }

        VertexBuffers.back()->Unmap();

        CommandList->SetRootSignature(Material->GetRootSignature());
        CommandList->SetResourceView(RHI::CstResourceViewType::Cbv, 0, PerFrameData);

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
        const LineArgs& Args,
        bool            WorldsSpace)
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

        CurVertex[0].Position        = Args.StartPosition;
        CurVertex[0].Color           = Args.StartColor;
        CurVertex[0].NeedsProjection = WorldsSpace;

        CurVertex[1].Position        = Args.EndPosition;
        CurVertex[1].Color           = Args.EndColor;
        CurVertex[1].NeedsProjection = WorldsSpace;
    }

    void DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::Append(
        float           Duration,
        const LineArgs& Args,
        bool            WorldsSpace)
    {
        m_TimedLines.emplace(float(GameEngine::Get()->GetEngineTime() + Duration), std::make_pair(Args, WorldsSpace));
    }

    void DefaultEngineDebugOverlay::Overlay_Debug_LineBuffer::Reset()
    {
        DrawCount = 0;
        VertexBuffers.clear();
    }
} // namespace Neon::Runtime

#endif // NEON_DIST