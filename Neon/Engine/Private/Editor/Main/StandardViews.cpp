#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Editor/Views/Types/ContentBrowser.hpp>

namespace Neon::Editor
{
    void EditorEngine::AddStandardViews()
    {
        RegisterView<Views::ContentBrowser>("_ContentBrowser");

        OpenView("_ContentBrowser");
    }
} // namespace Neon::Editor