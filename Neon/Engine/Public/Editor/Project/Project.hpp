#pragma once

#include <Core/Version.hpp>

#include <Asset/Handle.hpp>
#include <Asset/Packs/Directory.hpp>
#include <Asset/Types/RuntimeScene.hpp>

#include <filesystem>

namespace Neon::Editor
{
    struct ProjectConfig
    {
        StringU8    Name    = "New Project";
        VersionType Version = VersionType(1, 0, 0);

        Ptr<Asset::RuntimeSceneAsset> StartupScene;

        /// <summary>
        /// Default profile to use when no profile is selected.
        /// </summary>
        Asset::Handle CurrentProfile = Asset::Handle::FromString("1f212ba0-6313-4452-8dec-92b34f7b21e3");

        float AssemblyAutoReloadInterval = 5.f;
        float AutoSaveInterval           = 25.f;

        //

        Vector3 EditorCameraPosition = Vector3(0.f, 0.f, -25.f);
        Vector3 EditorCameraRotation = Vector3(0.f, 0.f, 0.f);

        float EditorCameraFOV   = 45.f;
        float EditorCameraSpeed = 5.f;

        //

        bool AssemblyAutoReload = true;
        bool AutoSave           = true;
    };

    class Project
    {
        friend class ProjectManager;

    public:
        Project(
            const std::filesystem::path& ProjectPath,
            bool                         LoadEmpty);
        NEON_CLASS_NO_COPY(Project);
        NEON_CLASS_NO_MOVE(Project);
        ~Project();

        /// <summary>
        /// Gets the current project.
        /// </summary>
        [[nodiscard]] static Project* Get();

        /// <summary>
        /// Gets the current project.
        /// </summary>
        [[nodiscard]] static const ProjectConfig& Config();

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

    public:
        /// <summary>
        /// Gets the content package.
        /// </summary>
        Asset::DirectoryAssetPackage* GetContentPackage() noexcept;

    public:
        /// <summary>
        /// Save an asset.
        /// </summary>
        void SaveAsset(
            const Ptr<Asset::IAsset>& Asset);

        /// <summary>
        /// Save an asset.
        /// </summary>
        template<typename _Ty>
            requires(std::derived_from<_Ty, Asset::IAsset> && !std::is_same_v<_Ty, Asset::IAsset>)
        void SaveAsset(
            const Ptr<_Ty>& Asset)
        {
            SaveAsset(std::static_pointer_cast<Asset::IAsset>(Asset));
        }

    public:
        /// <summary>
        /// Gets the active scene.
        /// </summary>
        [[nodiscard]] const Ptr<Asset::RuntimeSceneAsset>& GetActiveScene() const;

        /// <summary>
        /// Activates the current scene.
        /// </summary>
        void ActivateCurrentScene();

        /// <summary>
        /// Sets the active scene.
        /// </summary>
        void SetActiveScene(
            const Ptr<Asset::RuntimeSceneAsset>& Scene);

        /// <summary>
        /// Creates a new scene.
        /// </summary>
        void CreateNewScene();

        /// <summary>
        /// Mark the scene as dirty.
        /// </summary>
        void SceneDirty(
            bool State = true);

        /// <summary>
        /// Check if the scene is dirty.
        /// </summary>
        [[nodiscard]] bool IsSceneDirty() const;

    private:
        /// <summary>
        /// Loads the project.
        /// </summary>
        bool Load();

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

        Asset::DirectoryAssetPackage* m_ContentPackage;

        ProjectConfig m_Config;

        bool m_SceneIsDirty = false;
    };
} // namespace Neon::Editor