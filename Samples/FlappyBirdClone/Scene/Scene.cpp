#include "Engine.hpp"

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Physics.hpp>

using namespace Neon;

void FlappyBirdClone::LoadScene()
{
    auto& Scene = m_Runtime->GetScene();
    Scene.SetTimeScale(.2f);

    // Player instance
    auto PlayerSprite = Scene.CreateEntity(Scene::EntityType::Sprite, "PlayerSprite");
    {
        auto SpriteComponent = PlayerSprite.get_mut<Scene::Component::Sprite>();
        {
            SpriteComponent->MaterialInstance = GetMaterial("BaseSprite")->CreateInstance();
            SpriteComponent->MaterialInstance->SetTexture("p_SpriteTextures", m_Sprite);
        }

        auto TransformComponent = PlayerSprite.get_mut<Scene::Component::Transform>();
        {
            TransformComponent->World.SetPosition(Vec::Forward<Vector3> * 2.5f);
            TransformComponent->Local = TransformComponent->World;
        }

        {
            PlayerSprite.set(Scene::Component::CollisionShape{
                new btBoxShape(btVector3(5.f, 5.f, 5.f)) });
            Scene::Component::CollisionObject::AddRigidBody(PlayerSprite, 10.f);
        }
    }

    // Player camera
    auto Camera = Scene.CreateEntity(Scene::EntityType::Camera2D, "PlayerCamera");
    {
        Scene.GetEntityWorld()->set<Scene::Component::MainCamera>({ Camera });

        auto CameraComponent = Camera.get_mut<Scene::Component::Camera>();
        {
            CameraComponent->Viewport.OrthographicSize = 15.0f;
            CameraComponent->Viewport.NearPlane        = -1.0f;
            CameraComponent->Viewport.FarPlane         = 10.0f;

            CameraComponent->LookAt = PlayerSprite.get<Scene::Component::Transform>()->World.GetPosition();
        }
    }
}
