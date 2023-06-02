#include <EnginePCH.hpp>
#include <Resource/Runtime/Manager.hpp>

#include <Resource/Packs/ZipPack.hpp>

#include <Resource/Types/Logger.hpp>
#include <Resource/Types/TextFile.hpp>
#include <Resource/Types/Shader.hpp>

namespace Neon::Asset
{
    RuntimeResourceManager::RuntimeResourceManager()
    {
        AddHandler<LoggerAsset::Handler>();
        AddHandler<TextFileAsset::Handler>();
        AddHandler<ShaderAsset::Handler>();
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