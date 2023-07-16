#pragma once

#include <Asset/Handler.hpp>

namespace Neon::AAsset
{
    class TextFileHandler : public IAssetHandler
    {
    public:
        Asio::CoLazy<Ptr<IAsset>> Load(
            IO::InArchive2&       Archive,
            const AAsset::Handle& AssetGuid,
            AssetDependencyGraph& Graph) override;
    };
} // namespace Neon::AAsset