#include <PakCPCH.hpp>
#include <Parser/Manager.hpp>

#include <Resource/Packs/ZipPack.hpp>

#include <Resource/Types/Logger.hpp>
#include <Resource/Types/TextFile.hpp>
#include <Resource/Types/Shader.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <Resource/Types/Texture.hpp>

namespace PakC
{
    using namespace Neon::Asset;

    CustomResourceManager::CustomResourceManager()
    {
        AddHandler<LoggerAsset::Handler>();
        AddHandler<TextFileAsset::Handler>();
        AddHandler<ShaderLibraryAsset::Handler>();
        AddHandler<RootSignatureAsset::Handler>();
        AddHandler<TextureAsset::Handler>();
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