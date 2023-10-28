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

            Structured::LayoutBuilder Local;
            Local.Append(Structured::Type::Float3, "Color_Albedo");
            Local.Append(Structured::Type::Float3, "Color_Specular");
            Local.Append(Structured::Type::Float4, "Color_Emissive");

            LitMaterial.VarBuilder()
                .AddResource("Tex2D_Albedo", true)
                .AddResource("Tex2D_Specular", true)
                .AddResource("Tex2D_Emissive", true)
                .AddResource("Tex2D_Normal", true)
                .SetData(std::move(Local), true);

            LitMaterial
                .Topology(RHI::PrimitiveTopologyCategory::Triangle);

            LitMaterial
                .VertexShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex }))
                .PixelShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel }));

            auto Material = s_DefaultMaterials[Type::Lit] = LitMaterial.Build();
            Material->Set("Color_Albedo", Vector3{ 1.f, 1.f, 1.f });
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