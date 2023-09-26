#include <EnginePCH.hpp>
#include <Scene/Component/Sprite.hpp>
#include <RHI/Material/Shared.hpp>

#include <RHI/Resource/Resource.hpp>

namespace Neon::Scene::Component
{
    Sprite::Sprite()
    {
        MaterialInstance = RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::LitSprite)->CreateInstance();
    }
} // namespace Neon::Scene::Component