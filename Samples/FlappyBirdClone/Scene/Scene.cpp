#include "Engine.hpp"

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Physics.hpp>

#include <Input/Table.hpp>
#include <Input/System.hpp>

using namespace Neon;

void FlappyBirdClone::LoadScene()
{
    auto& Scene = m_Runtime->GetScene();

    // Player instance
    m_Player = Scene.CreateEntity(Scene::EntityType::Sprite, "PlayerSprite");
    {
        auto SpriteComponent = m_Player.get_mut<Scene::Component::Sprite>();
        {
            SpriteComponent->MaterialInstance = GetMaterial("BaseSprite")->CreateInstance();
            SpriteComponent->MaterialInstance->SetTexture("p_SpriteTextures", m_Sprite);
        }

        auto TransformComponent = m_Player.get_mut<Scene::Component::Transform>();
        {
            TransformComponent->World.SetPosition(Vec::Forward<Vector3> * 2.5f);
            TransformComponent->Local = TransformComponent->World;
        }

        {
            m_Player.set(Scene::Component::CollisionShape{
                new btBoxShape(btVector3(5.f, 5.f, 5.f)) });
            Scene::Component::CollisionObject::AddRigidBody(m_Player, 10.f);
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

            CameraComponent->LookAt = m_Player.get<Scene::Component::Transform>()->World.GetPosition();
        }
    }

    AttachInputs();
}

void FlappyBirdClone::AttachInputs()
{
    auto InputTable = Input::IInputDataTable::Create();
    GetWindow()->PushInputTable(InputTable);

    auto ActionTable = InputTable->LoadActionTable("Jump");
    ActionTable->Enable();

    auto SpaceAction = ActionTable->AddAction();
    SpaceAction->SetInput(Input::EKeyboardInput::Space);
    SpaceAction->Bind(
        Input::InputAction::BindType::Press,
        [this]()
        {
            auto CollisionObject = m_Player.get<Scene::Component::CollisionObject>()->BulletObject;
            if (auto RigidBody = btRigidBody::upcast(CollisionObject))
            {
                RigidBody->setLinearVelocity(Physics::ToBullet3(Vec::Up<Vector3> * 5.f));
            }
        });
}
