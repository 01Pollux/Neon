#include <EnginePCH.hpp>
#include <Renderer/Material/Builder.hpp>
#include <Renderer/Material/Material.hpp>

namespace Neon::Renderer
{
    Material::Material(
        const RenderMaterialBuilder& Builder)
    {
    }

    Material::Material(
        const ComputeMaterialBuilder& Builder)
    {
    }

    Ptr<MaterialInstance> Material::CreateInstance()
    {
        return m_DefaultInstace->CreateInstance();
    }

    //

    Ptr<MaterialInstance> MaterialInstance::CreateInstance()
    {
        return Ptr<MaterialInstance>();
    }
} // namespace Neon::Renderer