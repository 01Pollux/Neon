#include <EnginePCH.hpp>
#include <Scene/Exports/Export.hpp>

#include <Scene/Component/Transform.hpp>

#include <Scene/Component/Sprite.hpp>

#include <Scene/Component/Camera.hpp>

#include <Scene/Component/Physics.hpp>

namespace Neon::Scene::Component
{
    NEON_IMPLEMENT_COMPONENT(Transform)
    {
        World.component<Transform>("Neon::Scene::Transform");
    }

    //

    NEON_IMPLEMENT_COMPONENT(Sprite::MainRenderer)
    {
        World.component<MainRenderer>("Neon::Scene::Sprite::MainRenderer");
    }

    NEON_IMPLEMENT_COMPONENT(Sprite)
    {
        World.component<Sprite>("Neon::Scene::Sprite");
    }

    //

    NEON_IMPLEMENT_COMPONENT(Camera)
    {
        World.component<Camera>("Neon::Scene::Camera");
    }

    //

    NEON_IMPLEMENT_COMPONENT(CollisionEnter)
    {
        World.component<CollisionEnter>("Neon::Scene::CollisionEnter");
    }
    NEON_IMPLEMENT_COMPONENT(CollisionStay)
    {
        World.component<CollisionStay>("Neon::Scene::CollisionStay");
    }
    NEON_IMPLEMENT_COMPONENT(CollisionExit)
    {
        World.component<CollisionExit>("Neon::Scene::CollisionExit");
    }
    NEON_IMPLEMENT_COMPONENT(TriggerEnter)
    {
        World.component<TriggerEnter>("Neon::Scene::TriggerEnter");
    }
    NEON_IMPLEMENT_COMPONENT(TriggerStay)
    {
        World.component<TriggerStay>("Neon::Scene::TriggerStay");
    }
    NEON_IMPLEMENT_COMPONENT(TriggerExit)
    {
        World.component<TriggerExit>("Neon::Scene::TriggerExit");
    }

    //

    NEON_IMPLEMENT_COMPONENT(CollisionShape)
    {
        World.component<CollisionShape>("Neon::Scene::CollisionShape");
    }
    NEON_IMPLEMENT_COMPONENT(CollisionObject)
    {
        World.component<CollisionObject>("Neon::Scene::CollisionObject");
    }
} // namespace Neon::Scene::Component
