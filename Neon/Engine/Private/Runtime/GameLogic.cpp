#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>

#include <Renderer/Renderer.hpp>

namespace Neon::Runtime
{
    GameLogic::GameLogic(
        DefaultGameEngine* Engine) :
        m_Engine(Engine)
    {
    }

    GameLogic::~GameLogic() = default;

    void GameLogic::Tick()
    {
    }

    void GameLogic::Render()
    {
        if (m_Renderer)
        {
            m_Renderer->Render();
        }
    }
} // namespace Neon::Runtime