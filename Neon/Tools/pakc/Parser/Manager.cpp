#include <PakCPCH.hpp>
#include <Parser/Manager.hpp>

#include <Resource/Packs/ZipPack.hpp>

#include <Resource/Types/TextFile.hpp>

namespace PakC
{
    using namespace Neon::Asset;

    CustomResourceManager::CustomResourceManager()
    {
        AddHandler<TextFileAsset::Handler>();
    }

    Neon::UPtr<IAssetPack> CustomResourceManager::OpenPack(
        const Neon::StringU8& Path)
    {
        if (Path.ends_with(".np"))
        {
            return InstantiatePack<ZipAssetPack>();
        }
        return nullptr;
    }
} // namespace PakC