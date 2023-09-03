#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Project/Manager.hpp>

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
        if (m_CurrentProject)
        {
            m_CurrentProject->Unload();
            m_CurrentProject = nullptr;
        }

        m_CurrentProject = std::make_unique<Project>(ProjectPath);
        m_CurrentProject->LoadEmpty(std::move(Config));
    }

    Project* ProjectManager::GetActive() const
    {
        return m_CurrentProject.get();
    }
} // namespace Neon::Editor