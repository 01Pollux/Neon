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
    Runtime::DefaultGameEngine::Get()->SetTimeScale(1.f);
    auto& Scene = GetScene();

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
                std::make_unique<btCapsuleShape>(1.0f, 1.4f) });
            Scene::Component::CollisionObject::AddRigidBody(m_Player, 10.f);

            m_RigidBody = btRigidBody::upcast(m_Player.get<Scene::Component::CollisionObject>()->BulletObject.get());

            m_RigidBody->setAngularFactor(Physics::ToBullet3<true>(Vec::Forward<Vector3>));
            m_RigidBody->setLinearFactor(Physics::ToBullet3<true>(Vec::Up<Vector3>));

            m_Player.set<Scene::Component::CollisionEnter>({ std::bind(&FlappyBirdClone::OnCollisionEnter, this, std::placeholders::_1) });
        }
        m_Player.modified<Scene::Component::CollisionEnter>();
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

    // Stress test: Create 20'000 sprites
    if (1)
    {
        std::vector<Ptr<Renderer::IMaterial>> MaterialInstances;
        // Random 1000 material instances
        auto MaterialInstance = GetMaterial("BaseSprite");
        auto WorldTexture     = RHI::ITexture::GetDefault(RHI::DefaultTextures::White_2D);
        for (int i = 0; i < 1000; ++i)
        {
            auto Copy = MaterialInstance->CreateInstance();
            Copy->SetTexture("p_SpriteTextures", WorldTexture);
            MaterialInstances.emplace_back(Copy);
        }

        for (int i = 0; i < 200; ++i)
        {
            for (int j = 0; j < 1000; ++j)
            {
                auto Sprite = Scene.CreateEntity(Scene::EntityType::Sprite, StringUtils::Format("StressTestSprite{}{}", i, j).c_str());
                {
                    auto SpriteComponent = Sprite.get_mut<Scene::Component::Sprite>();
                    {
                        // Select random material instance
                        SpriteComponent->MaterialInstance = MaterialInstances[j];
                        // Random color based on i and j
                        SpriteComponent->ModulationColor = Color4(
                            ((i * 1000 + j) % 255) / 255.f,
                            ((i * 1000 + j) % 255) / 255.f,
                            ((i * 1000 + j) % 255) / 255.f,
                            1.f);
                        SpriteComponent->Size = { 1.f, 1.f };
                    }
                    Sprite.modified<Scene::Component::Sprite>();

                    auto TransformComponent = Sprite.get_mut<Scene::Component::Transform>();
                    {
                        TransformComponent->World.SetPosition(Vec::Right<Vector3> * (i * 1.1f));
                        TransformComponent->Local = TransformComponent->World;
                    }
                    Sprite.modified<Scene::Component::Transform>();
                }
            }
            break;
        }
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
                    std::make_unique<btBoxShape>(btVector3(100.f, 1.45f, 100.f)) });
                Scene::Component::CollisionObject::AddStaticBody(Floor);
            }
        }

        auto Ceiling = Scene.CreateEntity(Scene::EntityType::Sprite, "Ceiling");
        {
            auto SpriteComponent = Ceiling.get_mut<Scene::Component::Sprite>();
            {
                SpriteComponent->Size             = { 100.f, 5.35f };
                SpriteComponent->MaterialInstance = WorldMaterial;
            }
            Ceiling.modified<Scene::Component::Sprite>();

            auto TransformComponent = Ceiling.get_mut<Scene::Component::Transform>();
            {
                TransformComponent->World.SetPosition(Vec::Up<Vector3> * 6.5f);
                TransformComponent->Local = TransformComponent->World;
            }
            Ceiling.modified<Scene::Component::Transform>();

            {
                Ceiling.set(Scene::Component::CollisionShape{
                    std::make_unique<btBoxShape>(btVector3(100.f, 1.32f, 100.f)) });
                Scene::Component::CollisionObject::AddStaticBody(Ceiling);
            }
        }
    }

    //

    AttachInputs();

    //

    Scene.GetEntityWorld()
        ->system()
        .multi_threaded()
        .kind(flecs::OnUpdate)
        .iter([this](flecs::iter)
              { OnUpdate(); });
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
    // Player lost
    if (!m_RigidBody->getActivationState())
    {
        return;
    }

    float Mult        = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());
    float EnginePower = m_EnginePower * Mult;

    bool UpdateVelocity;
    if (m_IsJumping)
    {
        EnginePower *= 2.6f;
        if (m_VelocityAccum < 0.f)
        {
            EnginePower *= 3.5f;
        }
        UpdateVelocity = true;
    }
    else
    {
        UpdateVelocity = false;
        EnginePower *= -5.6f;
    }

    m_VelocityAccum = std::clamp(m_VelocityAccum + EnginePower, -m_EnginePower * 2, m_EnginePower * 2);

    if (UpdateVelocity)
    {
        m_RigidBody->setLinearVelocity(Physics::ToBullet3<true>(Vec::Up<Vector3> * m_VelocityAccum));
    }

    //

    float Angle = std::lerp(180.f, 0.f, (m_VelocityAccum + m_EnginePower * 2) / (m_EnginePower * 4));

    auto& Transform = m_RigidBody->getWorldTransform();
    Transform.setRotation(btQuaternion(btVector3(0, 0, 1), glm::radians(Angle)));
    m_RigidBody->setInterpolationWorldTransform(Transform);
}

void FlappyBirdClone::OnCollisionEnter(
    btPersistentManifold* Manifold)
{
    m_RigidBody->setActivationState(0);

    //

    auto Sprite             = m_Player.get_mut<Scene::Component::Sprite>();
    Sprite->ModulationColor = Colors::Red;
    m_Player.modified<Scene::Component::Sprite>();
}