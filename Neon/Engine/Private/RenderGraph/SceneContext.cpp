#include <EnginePCH.hpp>
#include <RenderGraph/SceneContext.hpp>
#include <Scene/EntityWorld.hpp>
#include <RHI/Material/Material.hpp>

namespace Component = Neon::Scene::Component;

namespace Neon::RG
{
    SceneContext::SceneContext(
        const GraphStorage& Storage) :
        m_Storage(Storage)
    {
    }

    void SceneContext::Render(
        RHI::ICommandList* CommandList,
        RenderType         Type) const
    {
    }
} // namespace Neon::RG