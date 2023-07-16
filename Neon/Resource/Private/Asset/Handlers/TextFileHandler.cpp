#include <ResourcePCH.hpp>
#include <Asset/Handlers/TextFileHandler.hpp>
#include <Asset/Types/StringFile.hpp>

namespace Neon::AAsset
{
    Asio::CoLazy<Ptr<IAsset>> TextFileHandler::Load(
        IO::InArchive2&       Archive,
        const AAsset::Handle& AssetGuid,
        AssetDependencyGraph& Graph)
    {
        co_return nullptr;
    }
} // namespace Neon::AAsset