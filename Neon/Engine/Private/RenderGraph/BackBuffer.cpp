#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/BackBuffer.hpp>

#include <RHI/Resource/State.hpp>
#include <RHI/Swapchain.hpp>

#include <Renderer/Material/Material.hpp>
#include <Renderer/Material/Builder.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

namespace Neon
{
    namespace AssetGuids
    {
        static inline auto CopyToTextureShaderGuid()
        {
            return Asset::Handle::FromString("d54f5bd2-3945-4e46-acfb-b31de1f08ad5");
        }
    } // namespace AssetGuids
} // namespace Neon

namespace Neon::RG
{
    BackBufferFinalizer::BackBufferFinalizer()
    {
        // TODO: Load from asset rather than hardcoding
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr CopyToTextureShader = Asset::Manager::Load(AssetGuids::CopyToTextureShaderGuid());

        m_Material =
            Renderer::RenderMaterialBuilder()
                .RootSignature(
                    RHI::RootSignatureBuilder()
                        .AddDescriptorTable(
                            RHI::RootDescriptorTable().AddSrvRange("p_CopySource", 0, 0, 1),
                            RHI::ShaderVisibility::Pixel)
                        .AddStandardSamplers()
                        .Build())
                .VertexShader(CopyToTextureShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex }))
                .PixelShader(CopyToTextureShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel }))
                .Topology(RHI::PrimitiveTopologyCategory::Triangle)
                .DepthStencil(Renderer::MaterialStates::DepthStencil::None)
                .RenderTarget(0, RHI::EResourceFormat::R8G8B8A8_UNorm)
                .Build();
    }

    void BackBufferFinalizer::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        RHI::GraphicsCommandList RenderCommandList(CommandList);

        auto  StateManager  = RHI::IResourceStateManager::Get();
        auto& SourceTexture = Storage.GetOutputImage();
        auto  BackBuffer    = RHI::ISwapchain::Get()->GetBackBuffer();

        // Transition to appropriate states
        {
            // Transition the backbuffer to a render target.
            StateManager->TransitionResource(
                BackBuffer,
                RHI::MResourceState::FromEnum(RHI::EResourceState::RenderTarget));

            // Transition the output image to a copy source.
            StateManager->TransitionResource(
                SourceTexture.Get().get(),
                RHI::MResourceState::FromEnum(RHI::EResourceState::PixelShaderResource));
        }

        // Flush the barriers.
        StateManager->FlushBarriers(CommandList);

        // Set viewport, scissor rect and render target view
        {
            auto& Size = RHI::ISwapchain::Get()->GetSize();

            RenderCommandList.SetViewport(
                ViewportF{
                    .Width    = float(Size.Width()),
                    .Height   = float(Size.Height()),
                    .MaxDepth = 1.f,
                });
            RenderCommandList.SetScissorRect(RectF(Vec::Zero<Vector2>, Size));

            auto View = RHI::ISwapchain::Get()->GetBackBufferView();

            RenderCommandList.ClearRtv(
                View,
                Colors::Magenta);

            RenderCommandList.SetRenderTargets(
                View, 1);
        }

        m_Material->SetTexture(
            "p_CopySource",
            SourceTexture.Get());

        m_Material->Apply(RenderCommandList);

        RenderCommandList.SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleStrip);
        RenderCommandList.Draw(RHI::DrawArgs{ .VertexCountPerInstance = 4 });

        // Transition the backbuffer to a present state.
        StateManager->TransitionResource(
            BackBuffer,
            RHI::MResourceState_Present);

        // Flush the barriers.
        StateManager->FlushBarriers(CommandList);
    }
} // namespace Neon::RG