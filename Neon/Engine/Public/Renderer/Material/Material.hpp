#pragma once

#include <Core/Neon.hpp>
#include <memory>

namespace Neon::Renderer
{
    class MaterialInstance;

    template<bool>
    class GenericMaterialBuilder;

    class Material : public std::enable_shared_from_this<Material>
    {
    public:
        Material(
            const GenericMaterialBuilder<true>& Builder);

        Material(
            const GenericMaterialBuilder<false>& Builder);

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] Ptr<MaterialInstance> CreateInstance();

    private:
        Ptr<MaterialInstance> m_DefaultInstace;
    };

    class MaterialInstance
    {
    public:
        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] Ptr<MaterialInstance> CreateInstance();

    private:
        Ptr<Material> m_Material;
    };
} // namespace Neon::Renderer