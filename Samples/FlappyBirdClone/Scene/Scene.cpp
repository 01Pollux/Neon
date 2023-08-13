#include "Engine.hpp"

#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Physics.hpp>

#include <Physics/World.hpp>

#include <Input/Table.hpp>
#include <Input/System.hpp>

#include <Runtime/DebugOverlay.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

using namespace Neon;

struct RainbowSprite
{
};

constexpr uint32_t Player_CollisionGroup = 1 << 0;
constexpr uint32_t Wall_CollisionGroup   = 1 << 1;

constexpr uint32_t Player_CollisionMask = Wall_CollisionGroup;
constexpr uint32_t Wall_CollisionMask   = Player_CollisionGroup;

void FlappyBirdClone::LoadScene()
{
    Runtime::DefaultGameEngine::Get()->SetTimeScale(1.f);
    auto& Scene = GetScene();
    auto& World = *Scene.GetEntityWorld();

    // Scene.GetPhysicsWorld()->SetDebugFlags(btIDebugDraw::DebugDrawModes::DBG_DrawWireframe);

    // Floor and ceiling as sprite
    {
        auto WorldMaterial = GetMaterial("BaseSprite")->CreateInstance();

        auto WorldTexture = RHI::ITexture::GetDefault(RHI::DefaultTextures::White_2D);
        WorldMaterial->SetTexture("p_SpriteTextures", WorldTexture);

        auto WallCreate =
            [&](const char* Name, const Vector3& Position)
        {
            auto Wall = World.entity(Name);
            {
                Wall.add<RainbowSprite>();

                Scene::Component::Sprite SpriteComponent;
                {
                    SpriteComponent.MaterialInstance = WorldMaterial;
                    SpriteComponent.SpriteSize       = Vector2(100.f, 2.35);
                }
                Wall.set(std::move(SpriteComponent));
                Wall.add<Scene::Component::Sprite::MainRenderer>();

                Scene::Component::Transform Transform;
                {
                    Transform.World.SetPosition(Position);
                }
                Wall.set(std::move(Transform));

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

    // Player instance
    m_Player = World.entity("Player");
    {
        Scene::Component::Sprite SpriteComponent;
        {
            SpriteComponent.MaterialInstance = GetMaterial("BaseSprite")->CreateInstance();
            SpriteComponent.MaterialInstance->SetTexture("p_SpriteTextures", m_Sprite);
            SpriteComponent.SpriteSize = { 4.f, 5.2f };
        }
        m_Player.set(std::move(SpriteComponent));
        m_Player.add<Scene::Component::Sprite::MainRenderer>();

        Scene::Component::Transform Transform;
        {
            Transform.World.SetRotationEuler(glm::radians(Vec::Forward<Vector3> * -90.f));
        }
        m_Player.set(std::move(Transform));

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
    }

    // Player camera
    auto Camera = World.entity("PlayerCamera");
    {
        Scene.GetEntityWorld()->set<Scene::Component::MainCamera>({ Camera });

        Scene::Component::Camera CameraComponent(Scene::Component::CameraType::Orthographic);
        {
            RG::CreateStandard2DRenderGraph(CameraComponent, Camera);

            CameraComponent.Viewport.OrthographicSize = 50.0f;
            CameraComponent.Viewport.NearPlane        = -1.0f;
            CameraComponent.Viewport.FarPlane         = 10.0f;

            CameraComponent.LookAt = m_Player.get<Scene::Component::Transform>()->World.GetPosition();
        }
        Camera.set(std::move(CameraComponent));

        Scene::Component::Transform TransformComponent;
        {
            TransformComponent.World.SetRotationEuler(glm::radians(Vec::Right<Vector3> * -90.f));
            TransformComponent.World.SetPosition(Vec::Backward<Vector3> * 10.f);
        }
        Camera.set(std::move(TransformComponent));
    }

    m_ObstacleMaterial = GetMaterial("BaseSprite")->CreateInstance();
    m_ObstacleMaterial->SetTexture("p_SpriteTextures", m_HdrTriangle);

    for (int i = 0; i < 3; i++)
    {
        CreateObstacle(World.get_world(), Vec::Right<Vector3> * 25.f * float(i));
    }

    //

    AttachInputs();

    //

    Scene.GetEntityWorld()
        ->system()
        .kind(flecs::OnUpdate)
        .iter([this](flecs::iter Iter)
              { OnUpdate(Iter); });

    Scene.GetEntityWorld()
        ->system<RainbowSprite>()
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
                m_ObstacleSpeed += 0.1f;
                printf("Obstacle speed: %f\n", m_ObstacleSpeed);
            });

        auto Unedit = ActionTable->AddAction();
        Unedit->SetInput(Input::EKeyboardInput::R);
        Unedit->Bind(
            Input::InputAction::BindType::Press,
            [this]
            {
                m_ObstacleSpeed -= 0.1f;
                printf("Obstacle speed: %f\n", m_ObstacleSpeed);
            });
    }
}

void FlappyBirdClone::OnUpdate(
    flecs::iter& Iter)
{
    // Player lost
    if (!m_RigidBody->getActivationState())
    {
        m_RigidBody->setLinearVelocity({});
        m_RigidBody->setAngularVelocity({});
        return;
    }

    UpdateInputs();
    UpdateObstacles(Iter);
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

void FlappyBirdClone::CreateObstacle(
    flecs::world   World,
    const Vector3& Pos)
{
    auto ObstacleCreate =
        [this, &World](const Vector3& Position, const Vector2& Size, bool Up)
    {
        auto Obstacle = World.entity();
        {
            Obstacle.add<RainbowSprite>();

            Scene::Component::Sprite SpriteComponent;
            {
                SpriteComponent.MaterialInstance = m_ObstacleMaterial;
                SpriteComponent.SpriteSize       = Vector2(23.42f, 11.7f) * Size;
            }
            Obstacle.set(std::move(SpriteComponent));
            Obstacle.add<Scene::Component::Sprite::MainRenderer>();

            Scene::Component::Transform TransformComponent;
            {
                TransformComponent.World.SetPosition(Position);
                if (Up)
                {
                    TransformComponent.World.SetRotationEuler(glm::radians(Vec::Forward<Vector3> * 180.f));
                }
            }
            Obstacle.set(std::move(TransformComponent));

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

                Obstacle.set<Scene::Component::CollisionShape>({ std::move(TriangleShape) });
                auto StaticBody = Scene::Component::CollisionObject::AddStaticBody(Obstacle, Wall_CollisionGroup, Wall_CollisionMask);
                StaticBody->setCustomDebugColor(Physics::ToBullet3(Colors::Black));
            }
        }

        return Obstacle;
    };

    auto SpawnPosition = Vec::Right<Vector3> * 20.f + Pos;

    m_Obstacles.push_back(ObstacleCreate(Vec::Down<Vector3> * 12.f + SpawnPosition, Vector2(0.6f, 2.f), false));
    m_Obstacles.push_back(ObstacleCreate(Vec::Up<Vector3> * 12.f + SpawnPosition, Vector2(0.6f, 2.f), true));
}

//

void FlappyBirdClone::UpdateInputs()
{
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

void FlappyBirdClone::UpdateObstacles(
    flecs::iter& Iter)
{
    auto DeltaTime   = float(Runtime::DefaultGameEngine::Get()->GetDeltaTime());
    bool RemoveFirst = false;
    for (auto& Entity : m_Obstacles)
    {
        auto  Rigidbody = Entity.get<Scene::Component::CollisionObject>()->AsRigidBody();
        auto& Transform = Rigidbody->getWorldTransform();

        if (Transform.getOrigin().x() < -40.f)
        {
            RemoveFirst = true;
            continue;
        }

        Transform.setOrigin(Transform.getOrigin() + btVector3(-m_ObstacleSpeed, 0.f, 0.f) * DeltaTime);
        Rigidbody->setInterpolationWorldTransform(Transform);
        Rigidbody->getMotionState()->setWorldTransform(Transform);
    }

    static bool once = false;
    if (RemoveFirst)
        if (!once)
        {
            once = true;
            /*         m_Obstacles[0].destruct();
                 m_Obstacles[1].destruct();
                 m_Obstacles.erase(m_Obstacles.begin(), m_Obstacles.begin() + 2);*/

            CreateObstacle(Iter.world(), Vec::Right<Vector3> * 25.f);
        }
}
