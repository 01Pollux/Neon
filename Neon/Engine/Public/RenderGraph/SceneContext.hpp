#pragma once

#include <RenderGraph/Common.hpp>

#include <Renderer/Render/BaseRenderer.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>

namespace Neon::RG
{
    class GraphStorage;

    class SceneContext
    {
    public:
        enum class RenderType : uint8_t
        {
            // Deferred,
            DepthPrepass
        };

        SceneContext(
            const GraphStorage& Storage);

        /// <summary>
        /// Dispatch the renderers depending on the type
        /// </summary>
        void Render(
            RHI::ICommandList* CommandList,
            RenderType         Type) const;

    private:
        const GraphStorage& m_Storage;

        flecs::query<
            Scene::Component::Transform,
            Scene::Component::MeshInstance>
            m_MeshQuery;
    };
} // namespace Neon::RG