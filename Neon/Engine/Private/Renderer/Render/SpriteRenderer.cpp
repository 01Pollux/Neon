#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteRenderer.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::Renderer
{
    SpriteRenderer::SpriteRenderer(
        uint32_t MaxSpritePerBatch) :
        m_SpriteBatch(MaxSpritePerBatch),
        m_SpriteQuery(
            Runtime::DefaultGameEngine::Get()
                ->GetScene()
                .GetEntityWorld()
                ->query_builder<Component::Transform, Component::Sprite>()
                .with<Component::Sprite::MainRenderer>()
                .term<Component::Transform>()
                .in()
                .term<Component::Sprite>()
                .in()
                .build())
    {
    }

    SpriteRenderer::~SpriteRenderer()
    {
        m_SpriteQuery.destruct();
    }

    void SpriteRenderer::Render(
        RHI::GpuResourceHandle     CameraBuffer,
        RHI::IGraphicsCommandList* CommandList)
    {
        if (m_SpriteQuery.is_true())
        {
            m_SpriteBatch.SetCameraBuffer(CameraBuffer);
            m_SpriteBatch.Begin(CommandList);
            m_SpriteQuery.iter(
                [this](flecs::iter&                Iter,
                       const Component::Transform* Transform,
                       const Component::Sprite*    Sprite)
                {
                    for (size_t i : Iter)
                    {
                        m_SpriteBatch.Draw(Transform[i], Sprite[i]);
                    }
                });
            m_SpriteBatch.End();
        }
    }
} // namespace Neon::Renderer