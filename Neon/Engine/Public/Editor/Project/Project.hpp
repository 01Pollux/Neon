#pragma once

#include <Core/Neon.hpp>
#include <filesystem>

namespace Neon::Asset
{
    class IAssetPackage;
} // namespace Neon::Asset

namespace Neon::Editor
{
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

    private:
        /// <summary>
        /// Loads the project.
        /// </summary>
        void Load();

        /// <summary>
        /// Unloads the project.
        /// </summary>
        void Unload();

    private:
        std::filesystem::path m_ProjectPath;

        Asset::IAssetPackage* m_ContentPackage;
    };
} // namespace Neon::Editor