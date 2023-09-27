#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

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
    CopyToTexturePass::CopyToTexturePass(
        CopyToTextureData Data) :
        RenderPass("CopyToTexturePass"),
        m_Data(std::move(Data))
    {
        // TODO: Load from asset rather than hardcoding
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr CopyToTextureShader(Asset::Manager::LoadAsync(AssetGuids::CopyToTextureShaderGuid()));

        m_CopyToRootSignature =
            RHI::RootSignatureBuilder(STR("CopyToTexturePass::RootSignature"))
                .AddDescriptorTable(
                    "InputTextures",
                    RHI::RootDescriptorTable()
                        .AddSrvRange("", 0, 0, 1),
                    RHI::ShaderVisibility::Pixel)
                .AddStandardSamplers()
                .Build();

        RHI::PipelineStateBuilderG PipeineStateBuilder{
            .RootSignature = m_CopyToRootSignature,
            .VertexShader  = CopyToTextureShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex }),
            .PixelShader   = CopyToTextureShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel }),
            .DepthStencil  = { .DepthEnable = false },
            .RTFormats     = { RHI::EResourceFormat::R8G8B8A8_UNorm },
            .Topology      = RHI::PrimitiveTopologyCategory::Triangle
        };

        m_CopyToPipeline[size_t(BlendMode::Opaque)] = PipeineStateBuilder.Build();

        PipeineStateBuilder.Blend.RenderTargets[0] = {
            .BlendEnable = true,
            .Src         = RHI::BlendTarget::SrcAlpha,
            .Dest        = RHI::BlendTarget::InvSrcAlpha,
            .OpSrc       = RHI::BlendOp::Add,
        };
        m_CopyToPipeline[size_t(BlendMode::AlphaBlend)] = PipeineStateBuilder.Build();

        PipeineStateBuilder.Blend.RenderTargets[0] = {
            .BlendEnable = true,
            .Src         = RHI::BlendTarget::One,
            .Dest        = RHI::BlendTarget::One,
            .OpSrc       = RHI::BlendOp::Add,
        };
        m_CopyToPipeline[size_t(BlendMode::Additive)] = PipeineStateBuilder.Build();
    }

    void CopyToTexturePass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.ReadTexture(m_Data.Source.CreateView(m_Data.ViewName), RG::ResourceReadAccess::PixelShader);
        Resolver.WriteRenderTarget(m_Data.Destination.CreateView(m_Data.ViewName));
    }

    void CopyToTexturePass::DispatchTyped(
        const GraphStorage&      Storage,
        RHI::GraphicsCommandList CommandList)
    {
        Ptr<RHI::IPipelineState> PipelineState;
        switch (m_Data.Blend)
        {
        case BlendMode::Opaque:
            PipelineState = m_CopyToPipeline[size_t(BlendMode::Opaque)];
            break;
        case BlendMode::AlphaBlend:
            PipelineState = m_CopyToPipeline[size_t(BlendMode::AlphaBlend)];
            break;
        case BlendMode::Additive:
            PipelineState = m_CopyToPipeline[size_t(BlendMode::Additive)];
            break;
        }

        RHI::CpuDescriptorHandle Resource;
        Storage.GetResourceView(m_Data.Source.CreateView(m_Data.ViewName), &Resource);

        auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(1);
        Descriptor->Copy(Descriptor.Offset, { Resource, 1 });

        CommandList.SetRootSignature(m_CopyToRootSignature);
        CommandList.SetPipelineState(PipelineState);
        CommandList.SetDescriptorTable(0, Descriptor.GetGpuHandle());
        CommandList.SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleStrip);
        CommandList.Draw(RHI::DrawArgs{ .VertexCountPerInstance = 4 });
    }
} // namespace Neon::RG