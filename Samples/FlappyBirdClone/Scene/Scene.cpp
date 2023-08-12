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

    Scene.GetPhysicsWorld()->SetDebugFlags(btIDebugDraw::DebugDrawModes::DBG_DrawWireframe);

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
    if (0)
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
                }
                Wall.modified<Scene::Component::Sprite>();

                auto TransformComponent = Wall.get_mut<Scene::Component::Transform>();
                {
                    TransformComponent->World.SetScale(Vector3(100.f, 5.35, 1.f));
                    TransformComponent->World.SetPosition(Position);
                }
                Wall.modified<Scene::Component::Transform>();

                {
                    Wall.set(Scene::Component::CollisionShape{
                        std::make_unique<btBoxShape>(btVector3(100.f, 1.45f, 100.f)) });

                    auto StaticBody = Scene::Component::CollisionObject::AddStaticBody(Wall, Wall_CollisionGroup, Wall_CollisionMask);
                    StaticBody->setCustomDebugColor(Physics::ToBullet3(Colors::Black));
                }
            }
        };

        WallCreate("Ceiling", Vec::Up<Vector3> * 6.5f);
        WallCreate("Floor", Vec::Down<Vector3> * 6.5f);
    }

    // Create triangle in top and bottom (seperated by 3.5f)
    if (0)
    {
        auto CreatePhysicsBody = [](flecs::entity Triangle)
        {
            std::array<btVector3, 3> Vertices = {
                btVector3(-4.0f, -2.6f, 100.f),
                btVector3(0.f, 6.6f, 100.f),
                btVector3(4.0f, -2.6f, 100.f),
            };

            auto TriangleShape = std::make_unique<btBoxShape>(btVector3(1.f, 1.f, 100.f));

            // auto TriangleShape = std::make_unique<btConvexHullShape>(&Vertices[0].x(), int(Vertices.size()), int(sizeof(Vertices[0])));

            Triangle.set(Scene::Component::CollisionShape{ std::move(TriangleShape) });
            Scene::Component::CollisionObject::AddStaticBody(Triangle, Wall_CollisionGroup, Wall_CollisionMask);
        };

        auto TriangleMaterial = GetMaterial("BaseSprite")->CreateInstance();
        TriangleMaterial->SetTexture("p_SpriteTextures", m_HdrTriangle);

        auto Triangle = Scene.CreateEntity(Scene::EntityType::Sprite, "Triangle01");
        {
            Triangle.add<RainbowSprite>();

            auto SpriteComponent = Triangle.get_mut<Scene::Component::Sprite>();
            {
                SpriteComponent->MaterialInstance = TriangleMaterial;
            }
            Triangle.modified<Scene::Component::Sprite>();

            auto TransformComponent = Triangle.get_mut<Scene::Component::Transform>();
            {
                TransformComponent->World.SetPosition(Vec::Down<Vector3> * 5.8f);
            }
            Triangle.modified<Scene::Component::Transform>();

            CreatePhysicsBody(Triangle);
        }

        // auto Triangle2 = Scene.CreateEntity(Scene::EntityType::Sprite, "Triangle02");
        //{
        //     Triangle2.add<RainbowSprite>();

        //    auto SpriteComponent = Triangle2.get_mut<Scene::Component::Sprite>();
        //    {
        //        SpriteComponent->Size             = { 2.6f, 4.0f };
        //        SpriteComponent->MaterialInstance = TriangleMaterial;
        //    }
        //    Triangle2.modified<Scene::Component::Sprite>();

        //    auto TransformComponent = Triangle2.get_mut<Scene::Component::Transform>();
        //    {
        //        Quaternion Rot = glm::angleAxis(glm::radians(180.f), Vec::Forward<Vector3>);
        //        TransformComponent->World.SetBasis(glm::toMat3(Rot));
        //        TransformComponent->World.SetPosition(Vec::Up<Vector3> * 2.6f);
        //        TransformComponent->Local = TransformComponent->World;
        //    }
        //    Triangle2.modified<Scene::Component::Transform>();

        //    CreatePhysicsBody(Triangle);
        //}
    }

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
            TransformComponent->World.SetRotationEuler(Vec::Forward<Vector3> * -90.f);
        }
        m_Player.modified<Scene::Component::Transform>();

        {
            m_Player.set(Scene::Component::CollisionShape{
                std::make_unique<btCapsuleShape>(1.0f, 1.4f) });

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
            CameraComponent->Viewport.OrthographicSize = 15.0f;
            CameraComponent->Viewport.NearPlane        = -1.0f;
            CameraComponent->Viewport.FarPlane         = 10.0f;

            CameraComponent->LookAt = m_Player.get<Scene::Component::Transform>()->World.GetPosition();
        }
        Camera.modified<Scene::Component::Camera>();

        auto TransformComponent = Camera.get_mut<Scene::Component::Transform>();
        {
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
                auto Ent       = GetScene().GetEntityWorld()->lookup("Triangle01");
                auto Transform = Ent.get_mut<Scene::Component::Transform>();

                Transform->World.SetPosition(Transform->World.GetPosition() + Vec::Up<Vector3> * 0.05f);

                Ent.modified<Scene::Component::Transform>();
            });

        auto Unedit = ActionTable->AddAction();
        Unedit->SetInput(Input::EKeyboardInput::R);
        Unedit->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                auto Ent       = GetScene().GetEntityWorld()->lookup("Triangle01");
                auto Transform = Ent.get_mut<Scene::Component::Transform>();

                Transform->World.SetPosition(Transform->World.GetPosition() - Vec::Up<Vector3> * 0.05f);

                Ent.modified<Scene::Component::Transform>();
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
        m_RigidBody->setLinearVelocity(Physics::ToBullet3(Vec::Up<Vector3> * m_VelocityAccum));
    }

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