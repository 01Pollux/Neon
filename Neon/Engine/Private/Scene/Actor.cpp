#include <EnginePCH.hpp>
#include <Scene/Actor.hpp>

namespace Neon::Scene
{
    Actor::Actor(
        flecs::entity Entity) :
        m_Entity(Entity)
    {
    }
} // namespace Neon::Scene