#include "Engine.hpp"
#include <Asset/Manager.hpp>

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
    using SpriteAssetPtr = Neon::Asset::AssetTaskPtr<Neon::Asset::TextureAsset>;

    // Preload the sprite texture
    SpriteAssetPtr SpriteTexture = Asset::Manager::Load(AssetGuids::SpriteGuid());

    m_Sprite = RHI::PendingResource(SpriteTexture->GetImageInfo()).Access<RHI::ITexture>(RHI::CommandQueueType::Graphics);
}