#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>

#include <RHI/GlobalDescriptors.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Neon::Renderer
{
    void SpriteBatcher::SetCameraBuffer(
        RHI::GpuResourceHandle Buffer)
    {
        m_CameraBuffer = Buffer;
    }

    void SpriteBatcher::OnBegin()
    {
        BatchBaseClass::OnBegin();

        m_PerObjectBuffer.Map();
    }

    void SpriteBatcher::OnDraw()
    {
        BatchBaseClass::OnDraw();

        auto FirstMaterial = m_MaterialInstances.GetFirstMaterial();

        FirstMaterial->SetResourceView(
            "g_SpriteData",
            m_PerObjectBuffer.GetHandle());

        FirstMaterial->SetResourceView(
            "g_FrameData",
            m_CameraBuffer);

        m_MaterialInstances.Bind(m_CommandList);
    }

    void SpriteBatcher::OnReset()
    {
        BatchBaseClass::OnReset();

        m_PerObjectBuffer.Reset();
    }

    void SpriteBatcher::OnEnd()
    {
        BatchBaseClass::OnEnd();

        m_PerObjectBuffer.Unmap();
        m_MaterialInstances.Reset();
    }

    void SpriteBatcher::Draw(
        const Scene::Component::Transform& Transform,
        const Scene::Component::Sprite&    Sprite)
    {
        if (!Sprite.MaterialInstance)
        {
            return;
        }

        if (!m_PerObjectBuffer.Reserve(sizeof(PerObjectData)))
        {
            Reset();
        }

        uint32_t SpriteIndex = uint32_t(m_PerObjectBuffer.CurSize / sizeof(PerObjectData));

        Vector2    HalfSize = Sprite.Size * 0.5f;
        std::array Vertices = {
            BatchSpriteVertex{ .Position = Vector2(-HalfSize.x, HalfSize.y), .TexCoord = Sprite.TextureRect.TopLeft(), .SpriteIndex = SpriteIndex },
            BatchSpriteVertex{ .Position = Vector2(HalfSize.x, HalfSize.y), .TexCoord = Sprite.TextureRect.TopRight(), .SpriteIndex = SpriteIndex },
            BatchSpriteVertex{ .Position = Vector2(HalfSize.x, -HalfSize.y), .TexCoord = Sprite.TextureRect.BottomRight(), .SpriteIndex = SpriteIndex },
            BatchSpriteVertex{ .Position = Vector2(-HalfSize.x, -HalfSize.y), .TexCoord = Sprite.TextureRect.BottomLeft(), .SpriteIndex = SpriteIndex }
        };
        DrawQuad<>(Vertices);

        auto ObjectData = m_PerObjectBuffer.AllocateData<PerObjectData>(1);

        ObjectData->World        = glm::transpose(Transform.World.ToMat4x4());
        ObjectData->Color        = Sprite.ModulationColor;
        ObjectData->TextureIndex = m_MaterialInstances.Append(Sprite.MaterialInstance.get());
    }
} // namespace Neon::Renderer