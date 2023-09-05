#include <EnginePCH.hpp>
#include <Renderer/Material/Shared.hpp>
#include <Renderer/Material/Builder.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>
#include <RHI/Resource/Resource.hpp>

#include <Log/Logger.hpp>

namespace Neon::Renderer
{
    static std::map<SharedMaterials::Type, Ptr<IMaterial>> s_DefaultMaterials;

    void SharedMaterials::Initialize()
    {
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        //

        const auto         LitSpriteShaderGuid = Asset::Handle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
        ShaderAssetTaskPtr LitSpriteShader(Asset::Manager::Load(LitSpriteShaderGuid));

        //

        // Unlit sprite
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
                .VertexShader(LitSpriteShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex, .Flags = Flags }))
                .PixelShader(LitSpriteShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel, .Flags = Flags }));

            auto Material = Renderer::IMaterial::Create(std::move(BaseSpriteMaterial));
            Material->SetTexture("p_SpriteTextures", RHI::ITexture::GetDefault(RHI::DefaultTextures::White_2D));

            s_DefaultMaterials[Type::LitSprite] = std::move(Material);
        }
    }

    void SharedMaterials::Shutdown()
    {
        s_DefaultMaterials.clear();
    }

    Ptr<IMaterial> SharedMaterials::Get(
        Type Ty)
    {
        NEON_ASSERT(s_DefaultMaterials.contains(Ty), "Default material not found!");
        return s_DefaultMaterials[Ty];
    }
} // namespace Neon::Renderer