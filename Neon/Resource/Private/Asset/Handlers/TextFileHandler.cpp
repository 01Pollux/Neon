#include <ResourcePCH.hpp>
#include <Asset/Handlers/TextFileHandler.hpp>
#include <Asset/Types/StringFile.hpp>

#include <boost/serialization/string.hpp>

namespace Neon::AAsset
{
    Asio::CoLazy<Ptr<IAsset>> TextFileHandler::Load(
        IO::InArchive2&       Archive,
        const AAsset::Handle& AssetGuid,
        AssetDependencyGraph& Graph)
    {
        StringU8 Text;
        Archive >> Text;

        co_return std::make_shared<StringFileAsset>(std::move(Text), AssetGuid);
    }

    void TextFileHandler::Save(
        IO::OutArchive2&   Archive,
        const Ptr<IAsset>& Asset)
    {
        Archive << static_cast<const StringFileAsset*>(Asset.get())->GetText();
    }
} // namespace Neon::AAsset