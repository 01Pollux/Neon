#include <CorePCH.hpp>
#include <World/World.hpp>

namespace Neon
{
    static std::mutex s_WorldMutex;

    World::World()
    {
        std::lock_guard WorldLock(s_WorldMutex);
        m_World = std::make_unique<flecs::world>();
    }

    World::~World()
    {
        std::lock_guard WorldLock(s_WorldMutex);
        m_World = nullptr;
    }
} // namespace Neon
