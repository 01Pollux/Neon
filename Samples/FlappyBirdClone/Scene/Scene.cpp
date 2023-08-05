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
    auto& Scene = GetScene();
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
                std::make_unique<btCapsuleShape>(2.f, 2.f) });
            Scene::Component::CollisionObject::AddRigidBody(m_Player, 10.f);

            m_RigidBody = btRigidBody::upcast(m_Player.get<Scene::Component::CollisionObject>()->BulletObject.get());

            m_RigidBody->setAngularFactor(Physics::ToBullet3(Vec::Forward<Vector3>));
            m_RigidBody->setLinearFactor(Physics::ToBullet3(Vec::Up<Vector3>));
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
        Camera.modified<Scene::Component::Camera>();

        auto TransformComponent = Camera.get_mut<Scene::Component::Transform>();
        {
            TransformComponent->World.SetPosition(Vec::Backward<Vector3> * 10.f);
            TransformComponent->Local = TransformComponent->World;
        }
        Camera.modified<Scene::Component::Transform>();
    }

    // Floor and ceiling as sprite
    {
        auto WorldMaterial = GetMaterial("BaseSprite")->CreateInstance();

        auto WorldTexture = RHI::ITexture::GetDefault(RHI::DefaultTextures::White_2D);
        WorldMaterial->SetTexture("p_SpriteTextures", WorldTexture);

        auto Floor = Scene.CreateEntity(Scene::EntityType::Sprite, "Floor");
        {
            auto SpriteComponent = Floor.get_mut<Scene::Component::Sprite>();
            {
                SpriteComponent->Size             = { 100.f, 5.35f };
                SpriteComponent->MaterialInstance = WorldMaterial;
            }
            Floor.modified<Scene::Component::Sprite>();

            auto TransformComponent = Floor.get_mut<Scene::Component::Transform>();
            {
                TransformComponent->World.SetPosition(Vec::Down<Vector3> * 6.5f);
                TransformComponent->Local = TransformComponent->World;
            }
            Floor.modified<Scene::Component::Transform>();

            {
                Floor.set(Scene::Component::CollisionShape{
                    std::make_unique<btBoxShape>(btVector3(100.f, 1.f, 100.f)) });
                Scene::Component::CollisionObject::AddStaticBody(Floor);
            }
        }
    }

    //

    AttachInputs();

    //

    // GetPipeline()->Attach(
    //     "Update",
    //     std::bind(&FlappyBirdClone::OnUpdate, this));
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
    {
        auto AddAction = ActionTable->AddAction();
        AddAction->SetInput(Input::EKeyboardInput::A);
        AddAction->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                // Find entity by name
                auto Floor  = GetScene().GetEntityWorld()->entity("Floor");
                auto Sprite = Floor.get_mut<Scene::Component::Sprite>();
                Sprite->Size.y += 0.05f;
                m_Player.modified<Scene::Component::Sprite>();
            });

        auto RemAction = ActionTable->AddAction();
        RemAction->SetInput(Input::EKeyboardInput::R);
        RemAction->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                auto Floor  = GetScene().GetEntityWorld()->entity("Floor");
                auto Sprite = Floor.get_mut<Scene::Component::Sprite>();
                Sprite->Size.y -= 0.05f;
                m_Player.modified<Scene::Component::Sprite>();
            });
    }
}

void FlappyBirdClone::OnUpdate()
{
    float Mult        = float(GetScene().GetDeltaTime());
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

    m_VelocityAccum = std::clamp(m_VelocityAccum + EnginePower, -m_EnginePower * 2, m_EnginePower * 2);
    m_RigidBody->setLinearVelocity(Physics::ToBullet3(Vec::Up<Vector3> * m_VelocityAccum));

    //

    float Angle = std::lerp(180.f, 0.f, (m_VelocityAccum + m_EnginePower * 2) / (m_EnginePower * 4));

    auto& Transform = m_RigidBody->getWorldTransform();
    Transform.setRotation(btQuaternion(btVector3(0, 0, 1), glm::radians(Angle)));
    m_RigidBody->setInterpolationWorldTransform(Transform);
}
