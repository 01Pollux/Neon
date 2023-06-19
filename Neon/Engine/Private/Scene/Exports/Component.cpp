#include <EnginePCH.hpp>
#include <Scene/Exports/Export.hpp>

#include <Scene/Component/CanvasItem.hpp>
#include <Scene/Component/Sprite.hpp>

namespace Neon::Scene::Component
{
    NEON_IMPLEMENT_COMPONENT(CanvasItem)
    {
        World.component<CanvasItem>("Neon::Scene::CanvasItem")
            .member<Color4>("ModulationColor")
            .member<RectI>("TextureRect");
    }

    NEON_IMPLEMENT_COMPONENT(Sprite)
    {
        World.component<Sprite>("Neon::Scene::Sprite")
            .is_a<CanvasItem>();
    }

    NEON_IMPLEMENT_COMPONENT(Sprite2)
    {
        World.component<Sprite2>("Neon::Scene::Sprite2")
            .is_a<CanvasItem>();
    }
} // namespace Neon::Scene::Component
