#include "Engine.hpp"

//

using namespace Neon;

namespace AssetGuids
{
    auto SpriteGuid()
    {
        return Asset::Handle::FromString("c01a7257-18ac-4cb3-ae8f-e989a824d9f2");
    }
} // namespace AssetGuids

//

void FlappyBirdClone::PreloadSprite()
{
    // Preload the sprite texture
    m_Sprite = Asset::Manager::Load(AssetGuids::SpriteGuid());
}