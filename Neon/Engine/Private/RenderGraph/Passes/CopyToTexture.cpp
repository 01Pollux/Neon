#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

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
    CopyToTexturePass::CopyToTexturePass(
        CopyToTextureData Data) :
        RenderPass(STR("CopyToTexturePass")),
        m_Data(std::move(Data))
    {
        // TODO: Load from asset rather than hardcoding
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr CopyToTextureShader = Asset::Manager::Load(AssetGuids::CopyToTextureShaderGuid());

        Renderer::RenderMaterialBuilder Builder;
        Builder.RootSignature(
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
            .RenderTarget(0, RHI::EResourceFormat::R8G8B8A8_UNorm);

        m_Materials[size_t(BlendMode::Opaque)] = Builder.Build();

        Builder.Blend(0, Renderer::MaterialStates::Blend::AlphaBlend);
        m_Materials[size_t(BlendMode::AlphaBlend)] = Builder.Build();

        Builder.Blend(0, Renderer::MaterialStates::Blend::Additive);
        m_Materials[size_t(BlendMode::Additive)] = Builder.Build();
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
        Ptr<Renderer::IMaterial> Material;
        switch (m_Data.Blend)
        {
        case BlendMode::Opaque:
            Material = m_Materials[size_t(BlendMode::Opaque)];
            break;
        case BlendMode::AlphaBlend:
            Material = m_Materials[size_t(BlendMode::AlphaBlend)];
            break;
        case BlendMode::Additive:
            Material = m_Materials[size_t(BlendMode::Additive)];
            break;
        }

        Material->SetTexture(
            "p_CopySource",
            Storage.GetResource(m_Data.Source).Get());

        Material->Apply(CommandList);

        CommandList.SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleStrip);
        CommandList.Draw(RHI::DrawArgs{ .VertexCountPerInstance = 4 });
    }
} // namespace Neon::RG