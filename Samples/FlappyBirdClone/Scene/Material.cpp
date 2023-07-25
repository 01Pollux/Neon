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
        Renderer::RenderMaterialBuilder BaseSpriteMaterial;

        BaseSpriteMaterial
            .Rasterizer(Renderer::MaterialStates::Rasterizer::CullNone)
            .DepthStencil(Renderer::MaterialStates::DepthStencil::None)
            .RenderTarget(0, "Base Color", RHI::EResourceFormat::R8G8B8A8_UNorm)
            .Topology(RHI::PrimitiveTopologyCategory::Triangle);

        auto& VarMap = BaseSpriteMaterial.VarMap();

        VarMap.Add("p_SpriteTextures", { 0, 0 }, Renderer::MaterialVarType::Resource)
            .Flags(Renderer::EMaterialVarFlags::Instanced, true);
        VarMap.Add("g_FrameData", { 0, 0 }, Renderer::MaterialVarType::Buffer);
        VarMap.Add("g_SpriteData", { 0, 1 }, Renderer::MaterialVarType::Resource);

        for (auto Type : std::ranges::iota_view(0ul, size_t(Renderer::MaterialStates::Sampler::_Last)))
        {
            VarMap.AddStaticSampler(
                StringUtils::Format("p_SpriteSamplers{}", Type),
                { Type, 0 },
                RHI::ShaderVisibility::Pixel,
                Renderer::MaterialStates::Sampler(Type));
        }

        BaseSpriteMaterial
            .VertexShader(RocketShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex }))
            .PixelShader(RocketShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel }));

        m_Materials.emplace(StringU8("BaseSprite"), Renderer::IMaterial::Create(std::move(BaseSpriteMaterial)));
    }
}
