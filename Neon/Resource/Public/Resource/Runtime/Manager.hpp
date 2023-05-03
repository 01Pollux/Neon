#pragma once

#include <Resource/Manager.hpp>

namespace Neon::Asset
{
    class RuntimeResourceManager : public IResourceManager
    {
    public:
        RuntimeResourceManager();

    protected:
        Ptr<IAssetPack> OpenPack(
            const StringU8& Path) override;
    };
} // namespace Neon::Asset