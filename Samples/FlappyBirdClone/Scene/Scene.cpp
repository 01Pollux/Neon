#include "Engine.hpp"

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Physics.hpp>

#include <Input/Table.hpp>
#include <Input/System.hpp>

#include <Runtime/DebugOverlay.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace Neon;

void FlappyBirdClone::LoadScene()
{
    auto& Scene = m_Runtime->GetScene();
    Scene.SetTimeScale(1.f);

    // Player instance
    m_Player = Scene.CreateEntity(Scene::EntityType::Sprite, "PlayerSprite");
    {
        auto SpriteComponent = m_Player.get_mut<Scene::Component::Sprite>();
        {
            SpriteComponent->MaterialInstance = GetMaterial("BaseSprite")->CreateInstance();
            SpriteComponent->MaterialInstance->SetTexture("p_SpriteTextures", m_Sprite);
        }
        m_Player.modified<Scene::Component::Sprite>();

        auto TransformComponent = m_Player.get_mut<Scene::Component::Transform>();
        {
            Quaternion Rot = glm::angleAxis(glm::radians(-90.f), Vec::Forward<Vector3>);
            TransformComponent->World.SetBasis(glm::toMat3(Rot));
            TransformComponent->Local = TransformComponent->World;
        }
        m_Player.modified<Scene::Component::Transform>();

        {
            m_Player.set(Scene::Component::CollisionShape{
                std::make_unique<btCapsuleShape>(8.f, 15.f) });
            Scene::Component::CollisionObject::AddRigidBody(m_Player, 10.f);

            m_RigidBody = btRigidBody::upcast(m_Player.get<Scene::Component::CollisionObject>()->BulletObject.get());

            m_RigidBody->setAngularFactor(Physics::ToBullet3(Vec::Forward<Vector3>));
            m_RigidBody->setLinearFactor(Physics::ToBullet3(Vec::Up<Vector3>));
        }
    }

    // Stress test: create 1024 sprites, each seperated by 1 unit in both x and y
    {
        constexpr uint32_t Count = 1024 - 1;

        // Start position is 50 units in the negative x direction and 25 units in the negative y direction
        Vector3 StartPosition = Vec::Left<Vector3> * 25.f + Vec::Down<Vector3> * 5.f;

        for (uint32_t i = 0; i < Count; ++i)
        {
            auto Name   = "Sprite" + std::to_string(i);
            auto Entity = Scene.CreateEntity(Scene::EntityType::Sprite, Name.c_str());
            {
                auto SpriteComponent = Entity.get_mut<Scene::Component::Sprite>();
                {
                    SpriteComponent->MaterialInstance = GetMaterial("BaseSprite")->CreateInstance();
                    SpriteComponent->MaterialInstance->SetTexture("p_SpriteTextures", m_Sprite);
                }
                Entity.modified<Scene::Component::Sprite>();

                // Seperated in both x and y, for each 50 units move to the next row
                auto TransformComponent = Entity.get_mut<Scene::Component::Transform>();
                {
                    TransformComponent->World.SetPosition((Vec::Right<Vector3> * float(i % 50) + Vec::Up<Vector3> * float(i / 50)) + StartPosition);
                    TransformComponent->Local = TransformComponent->World;
                }
            }
        }
    }

    // Player camera
    auto Camera = Scene.CreateEntity(Scene::EntityType::Camera3D, "PlayerCamera");
    {
        Scene.GetEntityWorld()->set<Scene::Component::MainCamera>({ Camera });

        auto CameraComponent = Camera.get_mut<Scene::Component::Camera>();
        {
            CameraComponent->Viewport.OrthographicSize = 15.0f;
            CameraComponent->Viewport.NearPlane        = -1.0f;
            CameraComponent->Viewport.FarPlane         = 10.0f;

            CameraComponent->LookAt = m_Player.get<Scene::Component::Transform>()->World.GetPosition();
        }
        Camera.modified<Scene::Component::Camera>();

        auto TransformComponent = Camera.get_mut<Scene::Component::Transform>();
        {
            TransformComponent->World.SetPosition(Vec::Backward<Vector3> * 10.f);
            TransformComponent->Local = TransformComponent->World;
        }
        Camera.modified<Scene::Component::Transform>();
    }

    AttachInputs();

    //

    GetPipeline()->Attach(
        "Update",
        std::bind(&FlappyBirdClone::OnUpdate, this));
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
        [this]
        {
            m_IsJumping = true;
        });
    SpaceAction->Bind(
        Input::InputAction::BindType::Release,
        [this]
        {
            m_RigidBody->setLinearVelocity({});
            m_IsJumping = false;
        });
}

void FlappyBirdClone::OnUpdate()
{
    /* Runtime::DebugOverlay::DrawCuboidLine(
         Vec::Forward<Vector3> * 5.f,
         Vec::One<Vector3> * 1.f,
         Colors::White);*/

    float Mult        = float(m_Runtime->GetScene().GetDeltaTime());
    float EnginePower = m_EnginePower * Mult;

    if (m_IsJumping)
    {
        EnginePower *= 1.6f;
        if (m_VelocityAccum < 0.f)
        {
            EnginePower *= 3;
        }
    }
    else
    {
        EnginePower *= -1.3f;
    }

    m_VelocityAccum += EnginePower;

    m_VelocityAccum = std::clamp(m_VelocityAccum, -m_EnginePower * 2, m_EnginePower * 2);

    m_RigidBody->setLinearVelocity(Physics::ToBullet3(Vec::Up<Vector3> * m_VelocityAccum));

    //

    float Angle = std::lerp(180.f, 0.f, (m_VelocityAccum + m_EnginePower * 2) / (m_EnginePower * 4));

    auto Transform = m_RigidBody->getWorldTransform();
    Transform.setRotation(btQuaternion(btVector3(0, 0, 1), glm::radians(Angle)));
    m_RigidBody->setWorldTransform(Transform);
    m_RigidBody->setInterpolationWorldTransform(Transform);
}
