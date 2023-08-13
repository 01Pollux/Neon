#include "Engine.hpp"

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Physics.hpp>

#include <Physics/World.hpp>

#include <Input/Table.hpp>
#include <Input/System.hpp>

#include <Runtime/DebugOverlay.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

using namespace Neon;

struct RainbowSprite
{
};

void FlappyBirdClone::LoadScene()
{
    constexpr uint32_t Player_CollisionGroup = 1 << 0;
    constexpr uint32_t Wall_CollisionGroup   = 1 << 1;

    constexpr uint32_t Player_CollisionMask = Wall_CollisionGroup;
    constexpr uint32_t Wall_CollisionMask   = Player_CollisionGroup;

    //

    Runtime::DefaultGameEngine::Get()->SetTimeScale(1.f);
    auto& Scene = GetScene();

    // Scene.GetPhysicsWorld()->SetDebugFlags(btIDebugDraw::DebugDrawModes::DBG_DrawWireframe);

    // Stress test: Create 200'000 sprites
    if (0)
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
            for (int j = 0; j < 100; ++j)
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
                    }
                    Sprite.modified<Scene::Component::Sprite>();

                    auto TransformComponent = Sprite.get_mut<Scene::Component::Transform>();
                    {
                        TransformComponent->World.SetPosition(Vec::Right<Vector3> * (i * 1.1f));
                    }
                    Sprite.modified<Scene::Component::Transform>();
                }
            }
        }
    }

    // Floor and ceiling as sprite
    {
        auto WorldMaterial = GetMaterial("BaseSprite")->CreateInstance();

        auto WorldTexture = RHI::ITexture::GetDefault(RHI::DefaultTextures::White_2D);
        WorldMaterial->SetTexture("p_SpriteTextures", WorldTexture);

        auto WallCreate =
            [&](const char* Name, const Vector3& Position)
        {
            auto Wall = Scene.CreateEntity(Scene::EntityType::Sprite, Name);
            {
                Wall.add<RainbowSprite>();

                auto SpriteComponent = Wall.get_mut<Scene::Component::Sprite>();
                {
                    SpriteComponent->MaterialInstance = WorldMaterial;
                    SpriteComponent->SpriteSize       = Vector2(100.f, 2.35);
                }
                Wall.modified<Scene::Component::Sprite>();

                auto TransformComponent = Wall.get_mut<Scene::Component::Transform>();
                {
                    TransformComponent->World.SetPosition(Position);
                }
                Wall.modified<Scene::Component::Transform>();

                {
                    Wall.set(Scene::Component::CollisionShape{
                        std::make_unique<btBoxShape>(btVector3(100.f, 2.35, 200.f) / 2.f) });

                    auto StaticBody = Scene::Component::CollisionObject::AddStaticBody(Wall, Wall_CollisionGroup, Wall_CollisionMask);
                    StaticBody->setCustomDebugColor(Physics::ToBullet3(Colors::Green));
                }
            }
        };

        WallCreate("Ceiling", Vec::Up<Vector3> * 14.f);
        WallCreate("Floor", Vec::Down<Vector3> * 14.f);
    }

    // Create triangle in top and bottom (seperated by 3.5f)
    {
        auto TriangleMaterial = GetMaterial("BaseSprite")->CreateInstance();
        TriangleMaterial->SetTexture("p_SpriteTextures", m_HdrTriangle);

        auto ObstacleCreate =
            [&](const char* Name, const Vector3& Position, const Vector2& Size, bool Up)
        {
            auto Obstacle = Scene.CreateEntity(Scene::EntityType::Sprite, Name);
            {
                Obstacle.add<RainbowSprite>();

                auto SpriteComponent = Obstacle.get_mut<Scene::Component::Sprite>();
                {
                    SpriteComponent->MaterialInstance = TriangleMaterial;
                    SpriteComponent->SpriteSize       = Vector2(23.42f, 11.7f) * Size;
                }
                Obstacle.modified<Scene::Component::Sprite>();

                auto TransformComponent = Obstacle.get_mut<Scene::Component::Transform>();
                {
                    TransformComponent->World.SetPosition(Position);
                    if (Up)
                    {
                        TransformComponent->World.SetRotationEuler(glm::radians(Vec::Forward<Vector3> * 180.f));
                    }
                }
                Obstacle.modified<Scene::Component::Transform>();

                {
                    std::array<btVector3, 3> Vertices = {
                        btVector3(-10.f, -4.75, -1.f),
                        btVector3(0.f, 4.75, -1.f),
                        btVector3(10.f, -4.75, -1.f),
                    };

                    for (auto& Vertex : Vertices)
                    {
                        Vertex.setX(Vertex.x() * Size.x);
                        Vertex.setY(Vertex.y() * Size.y);
                    }

                    auto TriangleShape = std::make_unique<btConvexHullShape>(&Vertices[0].x(), int(Vertices.size()), int(sizeof(Vertices[0])));

                    Obstacle.set(Scene::Component::CollisionShape{ std::move(TriangleShape) });
                    auto StaticBody = Scene::Component::CollisionObject::AddStaticBody(Obstacle, Wall_CollisionGroup, Wall_CollisionMask);
                    StaticBody->setCustomDebugColor(Physics::ToBullet3(Colors::Black));
                }
            }
        };

        ObstacleCreate("Obstacle01", Vec::Down<Vector3> * 12.f + Vec::Right<Vector3> * 5.f, Vector2(0.6f, 2.f), false);
        ObstacleCreate("Obstacle02", Vec::Up<Vector3> * 12.f + Vec::Right<Vector3> * 5.f, Vector2(0.6f, 2.f), true);
    }

    // Player instance
    m_Player = Scene.CreateEntity(Scene::EntityType::Sprite, "PlayerSprite");
    {
        auto SpriteComponent = m_Player.get_mut<Scene::Component::Sprite>();
        {
            SpriteComponent->MaterialInstance = GetMaterial("BaseSprite")->CreateInstance();
            SpriteComponent->MaterialInstance->SetTexture("p_SpriteTextures", m_Sprite);
            SpriteComponent->SpriteSize = { 4.f, 5.2f };
        }
        m_Player.modified<Scene::Component::Sprite>();

        auto TransformComponent = m_Player.get_mut<Scene::Component::Transform>();
        {
            TransformComponent->World.SetRotationEuler(glm::radians(Vec::Forward<Vector3> * -90.f));
        }
        m_Player.modified<Scene::Component::Transform>();

        {
            m_Player.set(Scene::Component::CollisionShape{
                std::make_unique<btCapsuleShape>(1.35f, 1.75f) });

            m_RigidBody = btRigidBody::upcast(
                Scene::Component::CollisionObject::AddRigidBody(m_Player, 10.f, Player_CollisionGroup, Player_CollisionMask));

            m_RigidBody->setAngularFactor(Physics::ToBullet3(Vec::Forward<Vector3>));
            m_RigidBody->setLinearFactor(Physics::ToBullet3(Vec::Up<Vector3>));
            m_RigidBody->setCustomDebugColor(Physics::ToBullet3(Colors::Red));

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
            CameraComponent->Viewport.OrthographicSize = 50.0f;
            CameraComponent->Viewport.NearPlane        = -1.0f;
            CameraComponent->Viewport.FarPlane         = 10.0f;

            CameraComponent->LookAt = m_Player.get<Scene::Component::Transform>()->World.GetPosition();
        }
        Camera.modified<Scene::Component::Camera>();

        auto TransformComponent = Camera.get_mut<Scene::Component::Transform>();
        {
            TransformComponent->World.SetRotationEuler(glm::radians(Vec::Right<Vector3> * -90.f));
            // TransformComponent->World.SetAxisAngle(Vec::Right<Vector3>, -90.f);
            TransformComponent->World.SetPosition(Vec::Backward<Vector3> * 10.f);
        }
        Camera.modified<Scene::Component::Transform>();
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

    Scene.GetEntityWorld()
        ->system<RainbowSprite>()
        .multi_threaded()
        .each(
            [this](flecs::entity Entity, RainbowSprite)
            {
                static float Time = 0.f;
                Time += 0.003f;

                Color4 Color;
                Color.r = (std::sin(Time * 2.f) + 1.f) / 2.f;
                Color.g = (std::sin(Time * 2.f + 2.f) + 1.f) / 2.f;
                Color.b = (std::sin(Time * 2.f + 4.f) + 1.f) / 2.f;
                Color.a = 1.f;

                auto SpriteComponent = Entity.get_mut<Scene::Component::Sprite>();

                SpriteComponent->ModulationColor = Color;

                Entity.modified<Scene::Component::Sprite>();
            });
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
        auto EditAction = ActionTable->AddAction();
        EditAction->SetInput(Input::EKeyboardInput::E);
        EditAction->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                auto Obstacle01 = GetScene().GetEntityWorld()->lookup("Obstacle01");
                auto Sprite     = Obstacle01.get_mut<Scene::Component::Sprite>();
                Sprite->SpriteSize.x += 0.1f;
                Obstacle01.modified<Scene::Component::Sprite>();
            });

        auto Unedit = ActionTable->AddAction();
        Unedit->SetInput(Input::EKeyboardInput::R);
        Unedit->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                auto Obstacle01 = GetScene().GetEntityWorld()->lookup("Obstacle01");
                auto Sprite     = Obstacle01.get_mut<Scene::Component::Sprite>();
                Sprite->SpriteSize.x -= 0.1f;
                Obstacle01.modified<Scene::Component::Sprite>();
            });
    }
    {
        auto EditAction = ActionTable->AddAction();
        EditAction->SetInput(Input::EKeyboardInput::T);
        EditAction->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                auto Obstacle01 = GetScene().GetEntityWorld()->lookup("Obstacle01");
                auto Sprite     = Obstacle01.get_mut<Scene::Component::Sprite>();
                Sprite->SpriteSize.y += 0.1f;
                Obstacle01.modified<Scene::Component::Sprite>();
            });

        auto Unedit = ActionTable->AddAction();
        Unedit->SetInput(Input::EKeyboardInput::Y);
        Unedit->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                auto Obstacle01 = GetScene().GetEntityWorld()->lookup("Obstacle01");
                auto Sprite     = Obstacle01.get_mut<Scene::Component::Sprite>();
                Sprite->SpriteSize.y -= 0.1f;
                Obstacle01.modified<Scene::Component::Sprite>();
            });
    }
}

void FlappyBirdClone::OnUpdate()
{
    // Player lost
    if (!m_RigidBody->getActivationState())
    {
        m_RigidBody->setLinearVelocity({});
        m_RigidBody->setAngularVelocity({});
        return;
    }

    float Mult        = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());
    float EnginePower = m_EnginePower * Mult;

    if (m_IsJumping)
    {
        EnginePower *= 2.6f;
        if (m_VelocityAccum < 0.f)
        {
            EnginePower *= 3.5f;
        }
    }
    else
    {
        EnginePower *= -4.6f;
    }

    m_VelocityAccum = std::clamp(m_VelocityAccum + EnginePower, -m_EnginePower * 2, m_EnginePower * 2);
    m_RigidBody->setLinearVelocity(Physics::ToBullet3(Vec::Up<Vector3> * m_VelocityAccum));

    //

    float Angle = std::lerp(-180.f, 0.f, (m_VelocityAccum + m_EnginePower * 2) / (m_EnginePower * 4));

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