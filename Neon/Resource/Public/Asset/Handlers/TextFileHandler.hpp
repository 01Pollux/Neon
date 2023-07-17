#pragma once

#include <Asset/Handler.hpp>

namespace Neon::AAsset
{
    class TextFileHandler : public IAssetHandler
    {
    public:
        static constexpr const char* HandlerName = "TextFile";

        Asio::CoLazy<Ptr<IAsset>> Load(
            IO::InArchive2&       Archive,
            const AAsset::Handle& AssetGuid,
            AssetDependencyGraph& Graph) override;

        void Save(
            IO::OutArchive2&   Archive,
            const Ptr<IAsset>& Asset) override;
    };
} // namespace Neon::AAsset