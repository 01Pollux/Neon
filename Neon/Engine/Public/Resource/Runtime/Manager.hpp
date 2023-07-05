#pragma once

#include <Resource/Manager.hpp>

namespace Neon::Asset
{
    class RuntimeAssetManager : public IAssetManager
    {
    public:
        RuntimeAssetManager();

    protected:
        Ptr<IAssetPack> OpenPack(
            const StringU8& Path) override;
    };
} // namespace Neon::Asset