#include <EnginePCH.hpp>
#include <Editor/Project/Manager.hpp>

#include <Asset/Packs/Directory.hpp>
#include <Asset/Storage.hpp>

#include <fstream>
#include <boost/property_tree/ini_parser.hpp>
#include <ZipLib/ZipFile.h>

#include <Log/Logger.hpp>

//

namespace bpt = boost::property_tree;

//

namespace Neon::Editor
{
    Project::Project(
        const std::filesystem::path& ProjectPath,
        bool                         LoadEmpty) :
        m_ProjectPath(ProjectPath)
    {
        if (LoadEmpty && std::filesystem::exists(ProjectPath))
        {
            std::filesystem::remove_all(ProjectPath);
        }

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

    const ProjectConfig& Project::GetConfig() const noexcept
    {
        return m_Config;
    }

    ProjectConfig& Project::GetConfig() noexcept
    {
        return m_Config;
    }

    //

    void Project::Save()
    {
        auto ProjectPath = GetProjectConfigPath();
        NEON_TRACE("Saving project: {}", ProjectPath.string());

        std::ofstream ConfigFile(ProjectPath);
        if (!ConfigFile.is_open())
        {
            NEON_ERROR("Failed to open project config file");
            return;
        }

        bpt::ptree Config;

        Config.put("Project.Name", m_Config.Name);
        Config.put("Project.Version", m_Config.Version.ToString());
        Config.put("Project.Start Scene", m_Config.StartupScene.ToString());

        Config.put("Project.Assembly Auto Reload", m_Config.AssemblyAutoReload);
        Config.put("Project.Assembly Auto Reload Interval", m_Config.AssemblyAutoReloadInterval);

        Config.put("Project.Auto Save", m_Config.AutoSave);
        Config.put("Project.Auto Save Interval", m_Config.AutoSaveInterval);

        const auto& Position = m_Config.EditorCameraPosition;
        Config.put("Editor.Camera Position", StringUtils::Format("{}, {}, {}", Position.x, Position.y, Position.z));

        const auto& Rotation = m_Config.EditorCameraRotation;
        Config.put("Editor.Camera Rotation", StringUtils::Format("{}, {}, {}", Rotation.x, Rotation.y, Rotation.z));

        bpt::write_ini(ConfigFile, Config);
        NEON_TRACE("Saved project: {} ({})", m_Config.Name, m_Config.Version.ToString());
    }

    bool Project::Load()
    {
        auto ProjectPath = GetProjectConfigPath();
        NEON_TRACE("Loading project: {}", ProjectPath.string());

        std::ifstream ConfigFile(ProjectPath);
        if (!ConfigFile.is_open())
        {
            NEON_ERROR("Failed to open project config file");
            return false;
        }

        bpt::ptree Config;
        bpt::read_ini(ConfigFile, Config);
        ConfigFile.close();

        m_Config.Name    = Config.get<std::string>("Project.Name", "Unnamed Project");
        m_Config.Version = Config.get<std::string>("Project.Version", "1.0.0");

        if (auto StartScene = Config.get_optional<std::string>("Project.Start Scene"))
        {
            m_Config.StartupScene = Asset::Handle::FromString(StartScene.get());
            NEON_TRACE("Startup scene: {}", m_Config.StartupScene.ToString());
        }
        else
        {
            NEON_TRACE("No startup scene specified");
        }

        m_Config.AssemblyAutoReload         = Config.get<bool>("Project.Assembly Auto Reload", true);
        m_Config.AssemblyAutoReloadInterval = Config.get<float>("Project.Assembly Auto Reload Interval", 1.0f);

        m_Config.AutoSave         = Config.get<bool>("Project.Auto Save", true);
        m_Config.AutoSaveInterval = Config.get<float>("Project.Auto Save Interval", 25.0f);

        StringU8 Tmp;
        Tmp = Config.get<StringU8>("Editor.Camera Position", "0.0, 0.0, 0.0");
        std::stringstream Stream(Tmp);

        auto& Position = m_Config.EditorCameraPosition;
        Stream >> Position.x;
        Stream.ignore();
        Stream >> Position.y;
        Stream.ignore();
        Stream >> Position.z;

        Tmp = Config.get<StringU8>("Editor.Camera Rotation", "0.0, 0.0, 0.0");
        Stream.str(Tmp);

        auto& Rotation = m_Config.EditorCameraRotation;
        Stream >> Rotation.x;
        Stream.ignore();
        Stream >> Rotation.y;
        Stream.ignore();
        Stream >> Rotation.z;

        NEON_TRACE("Loaded project: {} ({})", m_Config.Name, m_Config.Version.ToString());
        return true;
    }

    void Project::LoadEmpty(
        ProjectConfig Config)
    {
        NEON_TRACE("Loading empty project: {}", GetProjectDirectoryPath().string());
        m_Config = std::move(Config);

        //

        {
            auto File = ZipFile::Open("Templates/Empty.ntarch", false);

            for (auto& [EntryName, Entry] : File->GetEntries())
            {
                if (Entry->IsDirectory())
                {
                    continue;
                }

                auto& Stream = *Entry->GetDecompressionStream();
                auto  Path   = GetProjectDirectoryPath() / EntryName;

                if (auto ParentPath = Path.parent_path(); !std::filesystem::exists(ParentPath))
                    std::filesystem::create_directories(ParentPath);

                std::ofstream OutFile(Path, std::ios::binary | std::ios::trunc);
                if (!OutFile.is_open())
                {
                    NEON_ERROR("Failed to open file: {}", Path.string());
                    continue;
                }

                if (EntryName == "ProjectFiles/premake5.lua")
                {
                    std::string Line;
                    while (std::getline(Stream, Line))
                    {
                        StringUtils::Replace(Line, "PROJECT_NAME", m_Config.Name);
                        OutFile << Line << '\n';
                    }
                }
                else
                {
                    OutFile << Stream.rdbuf();
                }
                Entry->CloseDecompressionStream();
            }
        }

        Save();
        NEON_TRACE("Loaded project: {} ({})", m_Config.Name, m_Config.Version.ToString());
    }

    void Project::Unload()
    {
        NEON_TRACE("Unloading project: {} ({})", m_Config.Name, m_Config.Version.ToString());
        Save();
    }
} // namespace Neon::Editor