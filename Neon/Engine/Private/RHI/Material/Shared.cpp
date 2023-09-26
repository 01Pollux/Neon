#include <EnginePCH.hpp>
#include <RHI/Material/Shared.hpp>
#include <RHI/Material/Builder.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>
#include <RHI/Resource/Resource.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    static std::map<SharedMaterials::Type, Ptr<IMaterial>> s_DefaultMaterials;

    void SharedMaterials::Initialize()
    {
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        //

        const auto         LitShaderGuid = Asset::Handle::FromString("7fd1137c-ad31-4f83-8c35-6d2246f66bd2");
        ShaderAssetTaskPtr LitShader(Asset::Manager::LoadAsync(LitShaderGuid));

        const auto         LitSpriteShaderGuid = Asset::Handle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
        ShaderAssetTaskPtr LitSpriteShader(Asset::Manager::LoadAsync(LitSpriteShaderGuid));

        //

        auto& WhiteTexture = RHI::IGpuResource::GetDefaultTexture(RHI::DefaultTextures::White_2D);

        //

        // Lit
        {
            RHI::RenderMaterialBuilder LitMaterial;

            LitMaterial
                .Rasterizer(RHI::MaterialStates::Rasterizer::CullNone)
                .Topology(RHI::PrimitiveTopologyCategory::Triangle)
                .RootSignature(
                    RHI::RootSignatureBuilder(STR("Material::Lit_RootSignature"))
                        .AddConstantBufferView("Constant", 0, 0, RHI::ShaderVisibility::All)
                        .AddShaderResourceView("PerInstanceData", 0, 1, RHI::ShaderVisibility::Vertex)
                        .AddShaderResourceView("PerMaterialData", 0, 1, RHI::ShaderVisibility::Pixel)
                        .AddDescriptorTable(
                            "AlbedoMaps",
                            RHI::RootDescriptorTable(true).AddSrvRange("Data", 0, 2, 1), RHI::ShaderVisibility::Pixel)
                        .AddDescriptorTable(
                            "NormalMaps",
                            RHI::RootDescriptorTable(true).AddSrvRange("Data", 0, 3, 1), RHI::ShaderVisibility::Pixel)
                        .AddDescriptorTable(
                            "SpecularMaps",
                            RHI::RootDescriptorTable(true).AddSrvRange("Data", 0, 4, 1), RHI::ShaderVisibility::Pixel)
                        .AddDescriptorTable(
                            "EmissiveMaps",
                            RHI::RootDescriptorTable(true).AddSrvRange("Data", 0, 5, 1), RHI::ShaderVisibility::Pixel)
                        .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout)
                        .AddStandardSamplers()
                        .Build());

            RHI::MShaderCompileFlags Flags;
#if NEON_DEBUG
            Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

            LitMaterial
                .VertexShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex, .Flags = Flags }))
                .PixelShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel, .Flags = Flags }));

            s_DefaultMaterials[Type::Lit] = LitMaterial.Build();
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
} // namespace Neon::RHI