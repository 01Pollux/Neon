#pragma once

#include <RenderGraph/Graph.hpp>

namespace Neon::Renderer
{
    class IMaterial;
}

namespace Neon::RG
{
    class BackBufferFinalizer
    {
    public:
        BackBufferFinalizer();

        /// <summary>
        /// Sets the source texture to copy from.
        /// </summary>
        void SetSource(
            ResourceId Source);

        /// <summary>
        /// Dispatches and render to the back buffer.
        /// </summary>
        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList);

    private:
        ResourceId               m_SourceTexture;
        Ptr<Renderer::IMaterial> m_Material;
    };
} // namespace Neon::RG