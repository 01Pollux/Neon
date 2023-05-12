#pragma once

#include <Renderer/Material/Material.hpp>

namespace Renderer
{
    class UnlitMaterial : public Material
    {
    public:
        UnlitMaterial();

    private:
        /// <summary>
        /// Create the material meta data.
        /// </summary>
        void CreateMetaData();

        /// <summary>
        /// Create the material pipeline state.
        /// </summary>
        void CreatePipelineState();
    };
} // namespace Renderer