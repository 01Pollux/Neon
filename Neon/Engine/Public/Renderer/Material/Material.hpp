#pragma once

namespace Neon::Renderer
{
    template<bool>
    class GenericMaterialBuilder;

    class Material
    {
    public:
        Material(
            const GenericMaterialBuilder<true>& Builder);

        Material(
            const GenericMaterialBuilder<false>& Builder);
    };
} // namespace Neon::Renderer