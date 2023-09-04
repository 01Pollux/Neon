#include <EnginePCH.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Renderer/Material/Shared.hpp>

namespace Neon::Scene::Component
{
    Sprite::Sprite()
    {
        MaterialInstance = Renderer::SharedMaterials::Get(Renderer::SharedMaterials::Type::LitSprite)->CreateInstance();
    }
} // namespace Neon::Scene::Component