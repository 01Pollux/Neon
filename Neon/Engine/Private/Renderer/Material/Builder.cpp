#include <EnginePCH.hpp>
#include <Renderer/Material/Builder.hpp>

namespace Neon::Renderer
{
#define MATERIAL_SHADER_FUNC(MaterialClass, ShaderStage, Index)        \
    MaterialClass& MaterialClass::ShaderStage(                         \
        const RHI::ShaderMacros& Macros,                               \
        RHI::ShaderProfile       Profile,                              \
        RHI::MShaderCompileFlags Flags)                                \
    {                                                                  \
        m_ShaderModules[Index] = {                                     \
            .Macros  = Macros,                                         \
            .Profile = Profile,                                        \
            .Flags   = Flags,                                          \
        };                                                             \
        return *this;                                                  \
    }                                                                  \
                                                                       \
    auto MaterialClass::ShaderStage() const->const ShaderModuleHandle& \
    {                                                                  \
        return m_ShaderModules[Index];                                 \
    }

    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, VertexShader, 0);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, HullShader, 1);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, DomainShader, 2);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, GeometryShader, 3);
    MATERIAL_SHADER_FUNC(RenderMaterialBuilder, PixelShader, 4);

    MATERIAL_SHADER_FUNC(ComputeMaterialBuilder, ComputeShader, 0);

#undef MATERIAL_SHADER_FUNC

    //

    //

} // namespace Neon::Renderer