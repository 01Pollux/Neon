#pragma once

#include <Asset/Handle.hpp>

namespace Neon::RHI::Shaders
{
    static inline auto BlurShaderGuid()
    {
        return Asset::Handle::FromString("8d605fba-0199-4716-90ca-600366176e8a");
    }

    static inline auto CopyToTextureShaderGuid()
    {
        return Asset::Handle::FromString("d54f5bd2-3945-4e46-acfb-b31de1f08ad5");
    }

    static inline auto LitShaderGuid()
    {
        return Asset::Handle::FromString("7fd1137c-ad31-4f83-8c35-6d2246f66bd2");
    }

    static inline auto LitSpriteShaderGuid()
    {
        return Asset::Handle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
    }

    static inline auto GridFrustumGenShaderGuid()
    {
        return Asset::Handle::FromString("91f8ade0-a897-41c7-802c-e11cbec5861f");
    }

    static inline auto LightCullShaderGuid()
    {
        return Asset::Handle::FromString("de5df67e-e0c7-40dc-a73f-fc9ab93deca2");
    }

#ifndef NEON_DIST
    static inline auto DebugLineShaderGuid()
    {
        return Asset::Handle::FromString("bbd67e6b-afc3-4d55-bbda-ed382b3342f5");
    }
#endif
} // namespace Neon::RHI::Shaders