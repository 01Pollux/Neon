#include <ResourcePCH.hpp>
#include <Resource/Runtime/Manager.hpp>
#include <Private/Resource/ZipPack.hpp>

namespace Neon::Asset
{
    Ptr<IAssetPack> RuntimeResourceManager::OpenPack(
        const StringU8& Path,
        uint16_t        PackId)
    {
        if (Path.ends_with(".np"))
        {
            return std::make_shared<ZipAssetPack>(PackId);
        }
        return Ptr<IAssetPack>();
    }
} // namespace Neon::Asset