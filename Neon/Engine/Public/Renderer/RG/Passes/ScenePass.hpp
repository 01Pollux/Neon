#pragma once

#include <Renderer/RG/Pass.hpp>
#include <Renderer/RG/Graph.hpp>
#include <Renderer/Render/SpriteBatch.hpp>
#include <Renderer/Render/PrimitiveBatch.hpp>

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Scene.hpp>

namespace Neon::RG
{
    class ScenePass : public IRenderPass
    {
    public:
        ScenePass(
            const GraphStorage& Storage,
            Scene::GameScene&   Scene,
            Scene::Actor        Camera);

        void ResolveResources(
            ResourceResolver& Resolver) override;

        void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList) override;

    private:
        /// <summary>
        /// Update camera buffer.
        /// </summary>
        [[nodiscard]] Ptr<RHI::IUploadBuffer> UpdateCameraBuffer();

    private:
        UPtr<Renderer::SpriteBatch> m_SpriteBatch;
        Renderer::SpriteBatcher     m_SpriteBatch2;

        Scene::GameScene& m_Scene;
        Scene::Actor      m_Camera;

        flecs::query<
            Scene::Component::Transform,
            Scene::Component::Sprite>
            m_SpriteQuery;
    };
} // namespace Neon::RG