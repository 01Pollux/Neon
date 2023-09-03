#pragma once

#include <Core/String.hpp>
#include <Editor/Project/Project.hpp>

#include <filesystem>

namespace Neon::Editor
{
    class ProjectManager
    {
    public:
        /// <summary>
        /// Gets the project manager.
        /// </summary>
        [[nodiscard]] static ProjectManager* Get();

        /// <summary>
        /// Creates a new project.
        /// </summary>
        void NewEmptyProject(
            const std::filesystem::path& ProjectPath);

        /// <summary>
        /// Gets the current project.
        /// </summary>
        [[nodiscard]] Project* GetActive() const;

    private:
        UPtr<Project> m_CurrentProject = nullptr;
    };
} // namespace Neon::Editor