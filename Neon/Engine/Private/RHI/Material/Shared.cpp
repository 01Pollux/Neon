#include <EnginePCH.hpp>
#include <RHI/Material/Shared.hpp>
#include <RHI/Material/Builder.hpp>

#include <RHI/Shaders/Lit.hpp>
#include <RHI/Resource/Resource.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    static std::map<SharedMaterials::Type, Ptr<IMaterial>> s_DefaultMaterials;

    void SharedMaterials::Initialize()
    {
        RHI::Shaders::LitShader LitShader;

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
                        .AddDescriptorTable(
                            "TextureMaps",
                            RHI::RootDescriptorTable(true)
                                .AddSrvRange("Albedo", 0, 3, 1)
                                .AddSrvRange("Normal", 0, 3, 1)
                                .AddSrvRange("Specular", 0, 3, 1)
                                .AddSrvRange("Emissive", 0, 3, 1),
                            RHI::ShaderVisibility::Pixel)
                        .AddDescriptorTable(
                            "_LightData",
                            RHI::RootDescriptorTable()
                                .AddSrvRange("Data", 0, 1, 1)
                                .AddSrvRange("IndexList", 1, 1, 1)
                                .AddSrvRange("Grid", 2, 1, 1),
                            RHI::ShaderVisibility::Pixel)
                        .AddShaderResourceView("_PerInstanceData", 0, 2, RHI::ShaderVisibility::Vertex)
                        .AddShaderResourceView("_PerMaterialData", 0, 2, RHI::ShaderVisibility::Pixel)
                        .AddConstantBufferView("_FrameConstant", 0, 0, RHI::ShaderVisibility::All)
                        .SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout)
                        .AddStandardSamplers()
                        .Build());

            RHI::MShaderCompileFlags Flags;

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