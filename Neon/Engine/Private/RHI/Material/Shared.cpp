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
                .Topology(RHI::PrimitiveTopologyCategory::Triangle);

            LitMaterial
                .VertexShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Vertex }))
                .PixelShader(LitShader->LoadShader({ .Stage = RHI::ShaderStage::Pixel }));

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