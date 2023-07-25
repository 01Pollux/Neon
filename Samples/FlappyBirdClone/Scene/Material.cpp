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
        return Asset::Handle::FromString("c01a7257-18ac-4cb3-ae8f-e989a824d9f2");
    }
} // namespace AssetGuids

//

void FlappyBirdClone::PreloadMaterials()
{
    using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

    ShaderAssetTaskPtr RocketShader = Asset::Manager::Load(AssetGuids::RocketShaderGuid());

    {
        Renderer::RenderMaterialBuilder RocketMaterial;

        RocketMaterial
            .VertexShader(ShaderAsset->LoadShader({ .Stage = RHI::ShaderStage::Vertex }))
            .PixelShader(ShaderAsset->LoadShader({ .Stage = RHI::ShaderStage::Pixel }))
            .Rasterizer(Renderer::MaterialStates::Rasterizer::CullNone)
            .DepthStencil(Renderer::MaterialStates::DepthStencil::None)
            .RenderTarget(0, "Base Color", RHI::EResourceFormat::R8G8B8A8_UNorm)
            .Topology(RHI::PrimitiveTopologyCategory::Point);
    }

    // Preload the sprite material
    m_Materials["Sprite"] = Renderer::Material::Create(
        Renderer::Material::CreateInfo{
            .Name   = "Sprite",
            .Shader = Renderer::Shader::Create(
                Renderer::Shader::CreateInfo{
                    .Name     = "Sprite",
                    .Vertex   = Asset::Manager::Load(AssetGuids::SpriteVertexShaderGuid()),
                    .Fragment = Asset::Manager::Load(AssetGuids::SpriteFragmentShaderGuid()) }),
            .Uniforms = {
                Renderer::Material::Uniform{
                    .Name    = "u_Texture",
                    .Type    = Renderer::Material::UniformType::Texture2D,
                    .Texture = m_Sprite } } });
}
