#pragma once

#include <Renderer/Render/BaseRenderer.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>

namespace Neon::Renderer
{
    class SpriteRenderer : public IRenderer
    {
    public:
        SpriteRenderer(
            uint32_t MaxSpritePerBatch = 5000);

        NEON_CLASS_NO_COPYMOVE(SpriteRenderer);

        ~SpriteRenderer() override;

        /// <summary>
        /// Render all sprites in the scene
        /// </summary>
        void Render(
            RHI::GpuResourceHandle     CameraBuffer,
            RHI::IGraphicsCommandList* CommandList) override;

    private:
        Renderer::SpriteBatcher m_SpriteBatch;

        flecs::query<
            Scene::Component::Transform,
            Scene::Component::Sprite>
            m_SpriteQuery;
    };
} // namespace Neon::Renderer