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
        /// Dispatches and render to the back buffer.
        /// </summary>
        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList);

    private:
        Ptr<Renderer::IMaterial> m_Material;
    };
} // namespace Neon::RG