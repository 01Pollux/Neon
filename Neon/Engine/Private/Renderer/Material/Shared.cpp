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

        const auto         LitShaderGuid = Asset::Handle::FromString("7fd1137c-ad31-4f83-8c35-6d2246f66bd2");
        ShaderAssetTaskPtr LitShader(Asset::Manager::LoadAsync(LitShaderGuid));

        const auto         LitSpriteShaderGuid = Asset::Handle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
        ShaderAssetTaskPtr LitSpriteShader(Asset::Manager::LoadAsync(LitSpriteShaderGuid));

        //

        auto& WhiteTexture = RHI::IGpuResource::GetDefaultTexture(RHI::DefaultTextures::White_2D);

        //

        // Lit
        {
            Renderer::GBufferMaterialBuilder BaseSpriteMaterial;

            RHI::RootSignatureBuilder RsBuilder(STR("Material::Lit_RootSignature"));
            RsBuilder
                .AddConstantBufferView("g_CurrentData", 0, 1, RHI::ShaderVisibility::All)
                .AddConstantBufferView("g_FrameData", 0, 0, RHI::ShaderVisibility::All)
                .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout)
                .AddStandardSamplers();

            for (auto& [IsUav, SpaceSlot, Name] : {
                     std::tuple{ true, 48, "g_RW_Buffer[]" },
                     std::tuple{ false, 49, "g_Buffer[]" },

                     std::tuple{ true, 64, "g_RW_Texture1D_1f[]" },
                     std::tuple{ true, 65, "g_RW_Texture1D_2f[]" },
                     std::tuple{ true, 66, "g_RW_Texture1D_3f[]" },
                     std::tuple{ true, 67, "g_RW_Texture1D_4f[]" },

                     std::tuple{ true, 68, "g_RW_Texture1D_1i[]" },
                     std::tuple{ true, 69, "g_RW_Texture1D_2i[]" },
                     std::tuple{ true, 70, "g_RW_Texture1D_3i[]" },
                     std::tuple{ true, 71, "g_RW_Texture1D_4i[]" },

                     std::tuple{ true, 72, "g_RW_Texture1D_1u[]" },
                     std::tuple{ true, 73, "g_RW_Texture1D_2u[]" },
                     std::tuple{ true, 74, "g_RW_Texture1D_3u[]" },
                     std::tuple{ true, 75, "g_RW_Texture1D_4u[]" },

                     std::tuple{ false, 76, "g_Texture1D[]" },

                     std::tuple{ true, 77, "g_RW_Texture2D_1f[]" },
                     std::tuple{ true, 78, "g_RW_Texture2D_2f[]" },
                     std::tuple{ true, 79, "g_RW_Texture2D_3f[]" },
                     std::tuple{ true, 80, "g_RW_Texture2D_4f[]" },

                     std::tuple{ true, 81, "g_RW_Texture2D_1i[]" },
                     std::tuple{ true, 82, "g_RW_Texture2D_2i[]" },
                     std::tuple{ true, 83, "g_RW_Texture2D_3i[]" },
                     std::tuple{ true, 84, "g_RW_Texture2D_4i[]" },

                     std::tuple{ true, 85, "g_RW_Texture2D_1u[]" },
                     std::tuple{ true, 86, "g_RW_Texture2D_2u[]" },
                     std::tuple{ true, 87, "g_RW_Texture2D_3u[]" },
                     std::tuple{ true, 88, "g_RW_Texture2D_4u[]" },

                     std::tuple{ false, 89, "g_Texture2D[]" },

                     std::tuple{ true, 90, "g_Texture2DMS_1f[]" },
                     std::tuple{ true, 91, "g_Texture2DMS_2f[]" },
                     std::tuple{ true, 92, "g_Texture2DMS_3f[]" },
                     std::tuple{ true, 93, "g_Texture2DMS_4f[]" },

                     std::tuple{ true, 94, "g_Texture2DMS_1i[]" },
                     std::tuple{ true, 95, "g_Texture2DMS_2i[]" },
                     std::tuple{ true, 96, "g_Texture2DMS_3i[]" },
                     std::tuple{ true, 97, "g_Texture2DMS_4i[]" },

                     std::tuple{ true, 98, "g_Texture2DMS_1u[]" },
                     std::tuple{ true, 99, "g_Texture2DMS_2u[]" },
                     std::tuple{ true, 100, "g_Texture2DMS_3u[]" },
                     std::tuple{ true, 101, "g_Texture2DMS_4u[]" },

                     std::tuple{ true, 102, "g_RW_Texture3D_1f[]" },
                     std::tuple{ true, 103, "g_RW_Texture3D_2f[]" },
                     std::tuple{ true, 104, "g_RW_Texture3D_3f[]" },
                     std::tuple{ true, 105, "g_RW_Texture3D_4f[]" },

                     std::tuple{ true, 106, "g_RW_Texture3D_1i[]" },
                     std::tuple{ true, 107, "g_RW_Texture3D_2i[]" },
                     std::tuple{ true, 108, "g_RW_Texture3D_3i[]" },
                     std::tuple{ true, 109, "g_RW_Texture3D_4i[]" },

                     std::tuple{ true, 110, "g_RW_Texture3D_1u[]" },
                     std::tuple{ true, 111, "g_RW_Texture3D_2u[]" },
                     std::tuple{ true, 112, "g_RW_Texture3D_3u[]" },
                     std::tuple{ true, 113, "g_RW_Texture3D_4u[]" },

                     std::tuple{ false, 114, "g_Texture3D[]" },
                     std::tuple{ false, 115, "g_TextureCube[]" } })
            {
                if (IsUav)
                {
                    RsBuilder.AddDescriptorTable(
                        RHI::RootDescriptorTable()
                            .AddUavRange(Name, 0, SpaceSlot, std::numeric_limits<uint32_t>::max()),
                        RHI::ShaderVisibility::All);
                }
                else
                {
                    RsBuilder.AddDescriptorTable(
                        RHI::RootDescriptorTable()
                            .AddSrvRange(Name, 0, SpaceSlot, std::numeric_limits<uint32_t>::max()),
                        RHI::ShaderVisibility::All);
                }
            }

            RsBuilder.AddDescriptorTable(
                RHI::RootDescriptorTable()
                    .AddSamplerRange("g_Samplers", 0, 48, std::numeric_limits<uint32_t>::max()),
                RHI::ShaderVisibility::All);

            RsBuilder.AddDescriptorTable(
                RHI::RootDescriptorTable()
                    .AddSamplerRange("g_SamplersCmpState", 0, 49, std::numeric_limits<uint32_t>::max()),
                RHI::ShaderVisibility::All);

            BaseSpriteMaterial
                .Rasterizer(Renderer::MaterialStates::Rasterizer::CullNone)
                .Topology(RHI::PrimitiveTopologyCategory::Triangle)
                .RootSignature(RsBuilder.Build());

            RHI::MShaderCompileFlags Flags;
#if NEON_DEBUG
            Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

            BaseSpriteMaterial
                .VertexShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex, .Flags = Flags }))
                .PixelShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel, .Flags = Flags }));

            auto Material                 = Renderer::IMaterial::Create(std::move(BaseSpriteMaterial));
            s_DefaultMaterials[Type::Lit] = std::move(Material);
        }

        // Unlit sprite
        {
            Renderer::GBufferMaterialBuilder BaseSpriteMaterial;

            BaseSpriteMaterial
                .Rasterizer(Renderer::MaterialStates::Rasterizer::CullNone)
                .Topology(RHI::PrimitiveTopologyCategory::Triangle)
                .RootSignature(
                    RHI::RootSignatureBuilder(STR("Material::Sprite_RootSignature"))
                        .AddConstantBufferView("g_FrameData", 0, 0, RHI::ShaderVisibility::All)
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

            auto Material                       = Renderer::IMaterial::Create(std::move(BaseSpriteMaterial));
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