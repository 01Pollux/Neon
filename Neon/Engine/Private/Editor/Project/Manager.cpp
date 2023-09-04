#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Project/Manager.hpp>

#include <Log/Logger.hpp>

namespace Neon::Editor
{
    ProjectManager* ProjectManager::Get()
    {
        return EditorEngine::Get()->GetProjectManager();
    }

    void ProjectManager::NewEmptyProject(
        const std::filesystem::path& ProjectPath,
        ProjectConfig                Config)
    {
        // Currently we only support one project at a time, if we want to load another project, we will have to launch another instance of the editor
        NEON_ASSERT(!m_CurrentProject);
        m_CurrentProject = std::make_unique<Project>(ProjectPath, true);
        m_CurrentProject->LoadEmpty(std::move(Config));
    }

    bool ProjectManager::OpenProject(
        const std::filesystem::path& ProjectPath)
    {
        // Currently we only support one project at a time, if we want to load another project, we will have to launch another instance of the editor
        NEON_ASSERT(!m_CurrentProject);
        m_CurrentProject = std::make_unique<Project>(ProjectPath, false);
        if (!m_CurrentProject->Load())
        {
            m_CurrentProject = nullptr;
            return false;
        }
        return true;
    }

    Project* ProjectManager::GetActive() const
    {
        return m_CurrentProject.get();
    }
} // namespace Neon::Editor