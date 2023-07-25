#include "Engine.hpp"

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

using namespace Neon;

void FlappyBirdClone::LoadScene()
{
    auto& Scene = m_Runtime->GetScene();

    // Player instance
    {
        auto PlayerSprite = Scene.CreateEntity(Scene::EntityType::Sprite, "PlayerSprite");

        auto SpriteComponent = PlayerSprite.get_mut<Scene::Component::Sprite>();
        {
            SpriteComponent->MaterialInstance = GetMaterial("BaseSprite")->CreateInstance();
            SpriteComponent->MaterialInstance->SetTexture("p_SpriteTextures", m_Sprite);
        }
    }

    // Player camera
    {
        auto Camera = Scene.CreateEntity(Scene::EntityType::Camera2D, "PlayerCamera");

        Scene->set<Scene::Component::MainCamera>({ Camera });

        auto CameraComponent = Camera.get_mut<Scene::Component::Camera>();
        {
            CameraComponent->Viewport.OrthographicSize = 5.0f;
            CameraComponent->Viewport.NearPlane        = -1.0f;
            CameraComponent->Viewport.FarPlane         = 1.0f;
        }
    }
}
