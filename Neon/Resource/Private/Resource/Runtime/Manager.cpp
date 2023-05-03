#include <ResourcePCH.hpp>
#include <Resource/Runtime/Manager.hpp>

#include <Resource/Packs/ZipPack.hpp>

#include <Resource/Types/TextFile.hpp>

namespace Neon::Asset
{
    RuntimeResourceManager::RuntimeResourceManager()
    {
        AddHandler<TextFileAsset::Handler>();
    }

    UPtr<IAssetPack> RuntimeResourceManager::OpenPack(
        const StringU8& Path)
    {
        if (Path.ends_with(".np"))
        {
            return InstantiatePack<ZipAssetPack>();
        }
        return nullptr;
    }
} // namespace Neon::Asset