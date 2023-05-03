#include <ResourcePCH.hpp>
#include <Resource/Runtime/Manager.hpp>

#include <Private/Resource/ZipPack.hpp>

#include <Resource/Types/TextFile.hpp>

namespace Neon::Asset
{
    RuntimeResourceManager::RuntimeResourceManager()
    {
        AddHandler<TextFileAsset::Handler>();
    }

    Ptr<IAssetPack> RuntimeResourceManager::OpenPack(
        const StringU8& Path)
    {
        if (Path.ends_with(".np"))
        {
            return std::make_shared<ZipAssetPack>();
        }
        return nullptr;
    }
} // namespace Neon::Asset