#pragma once

#include <Renderer/Material/VariableMap.hpp>

namespace Neon::Renderer
{
    namespace MaterialImpl
    {
        class GenericMaterialBuilder
        {
        public:
            /// <summary>
            /// Get the variable map.
            /// </summary>
            [[nodiscard]] MaterialVariableMap& VarMap()
            {
                return m_VarMap;
            }

        private:
            MaterialVariableMap m_VarMap;
        };

        template<bool _IsCompute>
        class MaterialBuilder;

        template<>
        class MaterialBuilder<false> : public GenericMaterialBuilder
        {
        };

        template<>
        class MaterialBuilder<true> : public GenericMaterialBuilder
        {
        };
    } // namespace MaterialImpl

    using RenderMaterial  = MaterialImpl::MaterialBuilder<false>;
    using ComputeMaterial = MaterialImpl::MaterialBuilder<true>;
} // namespace Neon::Renderer