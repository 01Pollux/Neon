#pragma once

#include <Asset/Handler.hpp>

namespace Neon::AAsset
{
    class TextFileHandler : public IAssetHandler
    {
    public:
        Ptr<IAsset> Load(
            IO::InArchive2&       Archive,
            AssetDependencyGraph& Graph);
    };
} // namespace Neon::AAsset