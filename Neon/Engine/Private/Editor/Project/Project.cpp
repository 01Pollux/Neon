#include <EnginePCH.hpp>
#include <Editor/Project/Manager.hpp>

#include <Asset/Packs/Directory.hpp>
#include <Asset/Storage.hpp>

namespace Neon::Editor
{
    Project::Project(
        const std::filesystem::path& ProjectPath) :
        m_ProjectPath(ProjectPath)
    {
        auto ContentPackage = std::make_unique<Asset::DirectoryAssetPackage>(GetContentDirectoryPath());
        m_ContentPackage    = ContentPackage.get();
        Asset::Storage::Mount(std::move(ContentPackage));
    }

    Project::~Project()
    {
        Asset::Storage::Unmount(m_ContentPackage);
        m_ContentPackage = nullptr;
    }

    Project* Project::Get()
    {
        return ProjectManager::Get()->GetActive();
    }

    //

    const std::filesystem::path& Project::GetProjectDirectoryPath() const
    {
        return m_ProjectPath;
    }

    std::filesystem::path Project::GetProjectConfigPath() const
    {
        return GetProjectDirectoryPath() / "Project.neon";
    }

    std::filesystem::path Project::GetContentDirectoryPath() const
    {
        return GetProjectDirectoryPath() / "Content";
    }

    //

    void Project::Load()
    {
    }

    void Project::Unload()
    {
    }
} // namespace Neon::Editor