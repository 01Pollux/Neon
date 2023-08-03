#include "Engine.hpp"
#include <Asset/Manager.hpp>
#include <RHI/Resource/State.hpp>

//

using namespace Neon;

namespace AssetGuids
{
    auto RocketSpriteGuid()
    {
        return Asset::Handle::FromString("c01a7257-18ac-4cb3-ae8f-e989a824d9f2");
    }
    auto HdrTriangleSpriteGuid()
    {
        return Asset::Handle::FromString("c01a7257-18ac-4cb3-ae8f-e989a824d9f1");
    }
} // namespace AssetGuids

//

void FlappyBirdClone::PreloadSprite()
{
    using SpriteAssetPtr = Neon::Asset::AssetTaskPtr<Neon::Asset::TextureAsset>;

    // Preload the sprite texture
    SpriteAssetPtr SpriteTexture      = Asset::Manager::Load(AssetGuids::RocketSpriteGuid());
    SpriteAssetPtr HdrTriangleTexture = Asset::Manager::Load(AssetGuids::HdrTriangleSpriteGuid());

    m_Sprite      = RHI::PendingResource(SpriteTexture->GetImageInfo()).Access<RHI::ITexture>(RHI::CommandQueueType::Graphics);
    m_HdrTriangle = RHI::PendingResource(HdrTriangleTexture->GetImageInfo()).Access<RHI::ITexture>(RHI::CommandQueueType::Graphics);

    RHI::IResourceStateManager::Get()->TransitionResource(
        m_Sprite.get(),
        RHI::EResourceState::PixelShaderResource);

    RHI::IResourceStateManager::Get()->TransitionResource(
        m_HdrTriangle.get(),
        RHI::EResourceState::PixelShaderResource);
}