#include <EditorPCH.hpp>
#include <EditorMain/EditorEngine.hpp>
#include <Asset/Packs/Directory.hpp>

namespace Neon::Editor
{
    void EditorEngine::Initialize(
        Config::EngineConfig Config)
    {
        Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
        DefaultGameEngine::Initialize(std::move(Config));
    }
} // namespace Neon::Editor
