#pragma once

#include <Resource/Manager.hpp>

namespace Neon::Asset
{
    class RuntimeResourceManager : public IResourceManager
    {
    public:
    protected:
        Ptr<IAssetPack> OpenPack(
            const StringU8& Path,
            uint16_t        PackId) override;
    };
} // namespace Neon::Asset