#include <ResourcePCH.hpp>
#include <Asset/Handlers/TextFileHandler.hpp>
#include <Asset/Types/StringFile.hpp>

namespace Neon::AAsset
{
    Ptr<IAsset> TextFileHandler::Load(
        IO::InArchive2&       Archive,
        AssetDependencyGraph& Graph)
    {
        Ptr<StringFileAsset> TextAsset;

        Handle ThisGuid, ChildGuid;
        Archive >> ThisGuid >> ChildGuid;

        Ptr<IAsset> ChildAsset;

        Graph.Requires(
            ThisGuid,
            ChildGuid,
            ChildAsset);

        return nullptr;
    }
} // namespace Neon::AAsset