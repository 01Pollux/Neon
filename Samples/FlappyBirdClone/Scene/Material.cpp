#include "Engine.hpp"
#include <Renderer/Material/Builder.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

//

using namespace Neon;

namespace AssetGuids
{
    auto RocketShaderGuid()
    {
        return Asset::Handle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
    }
} // namespace AssetGuids

//

void FlappyBirdClone::PreloadMaterials()
{
    using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

    ShaderAssetTaskPtr RocketShader = Asset::Manager::Load(AssetGuids::RocketShaderGuid());

    {
        Renderer::GBufferMaterialBuilder BaseSpriteMaterial;

        BaseSpriteMaterial
            .Rasterizer(Renderer::MaterialStates::Rasterizer::CullNone)
            .Topology(RHI::PrimitiveTopologyCategory::Triangle)
            .RootSignature(
                RHI::RootSignatureBuilder()
                    .AddConstantBufferView("g_FrameData", 0, 1, RHI::ShaderVisibility::All)
                    .AddShaderResourceView("g_SpriteData", 0, 1, RHI::ShaderVisibility::All)
                    .AddDescriptorTable(
                        RHI::RootDescriptorTable()
                            .AddSrvRange("p_SpriteTextures", 0, 2, 1, true),
                        RHI::ShaderVisibility::Pixel)
                    .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout)
                    .AddStandardSamplers()
                    .Build());

        RHI::MShaderCompileFlags Flags;
#if NEON_DEBUG
        Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

        BaseSpriteMaterial
            .VertexShader(RocketShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex, .Flags = Flags }))
            .PixelShader(RocketShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel, .Flags = Flags }));

        m_Materials.emplace(StringU8("BaseSprite"), Renderer::IMaterial::Create(std::move(BaseSpriteMaterial)));
    }
}
