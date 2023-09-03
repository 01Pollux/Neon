#pragma once

#include <Core/Version.hpp>
#include <Asset/Handle.hpp>
#include <filesystem>

namespace Neon::Asset
{
    class IAssetPackage;
} // namespace Neon::Asset

namespace Neon::Editor
{
    struct ProjectConfig
    {
        StringU8    Name    = "New Project";
        VersionType Version = VersionType(1, 0, 0);

        Asset::Handle StartupScene;

        float AssemblyAutoReloadInterval = 5.f;
        float AutoSaveInterval           = 25.f;

        Vector3 EditorCameraPosition = Vector3(0.f, 0.f, 5.f);
        Vector3 EditorCameraRotation = Vector3(0.f, 0.f, 0.f);

        bool AssemblyAutoReload = true;
        bool AutoSave           = true;
    };

    class Project
    {
        friend class ProjectManager;

    public:
        Project(
            const std::filesystem::path& ProjectPath);
        NEON_CLASS_NO_COPY(Project);
        NEON_CLASS_NO_MOVE(Project);
        ~Project();

        /// <summary>
        /// Gets the current project.
        /// </summary>
        [[nodiscard]] static Project* Get();

    public:
        /// <summary>
        /// Save the project config file.
        /// </summary>
        void Save();

    public:
        /// <summary>
        /// Gets the project directory path. ("./")
        /// </summary>
        [[nodiscard]] const std::filesystem::path& GetProjectDirectoryPath() const;

        /// <summary>
        /// Get the project config path. ("./Project.neon")
        /// </summary>
        [[nodiscard]] std::filesystem::path GetProjectConfigPath() const;

        /// <summary>
        /// Gets the content directory path. ("./Content")
        /// </summary>
        [[nodiscard]] std::filesystem::path GetContentDirectoryPath() const;

    public:
        /// <summary>
        /// Gets the project config.
        /// </summary>
        [[nodiscard]] const ProjectConfig& GetConfig() const noexcept;

        /// <summary>
        /// Gets the project config.
        /// </summary>
        [[nodiscard]] ProjectConfig& GetConfig() noexcept;

    private:
        /// <summary>
        /// Loads the project.
        /// </summary>
        void Load();

        /// <summary>
        /// Loads an empty project.
        /// </summary>
        void LoadEmpty(
            ProjectConfig Config);

        /// <summary>
        /// Unloads the project.
        /// </summary>
        void Unload();

    private:
        std::filesystem::path m_ProjectPath;

        Asset::IAssetPackage* m_ContentPackage;

        ProjectConfig m_Config;
    };
} // namespace Neon::Editor