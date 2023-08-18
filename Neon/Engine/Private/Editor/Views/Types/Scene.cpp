#include <EnginePCH.hpp>
#include <Editor/Views/Types/Scene.hpp>

namespace Neon::Editor::Views
{
    Scene::Scene() :
        IEditorView(StandardViews::s_SceneViewWidgetId)
    {
    }

    void Scene::OnUpdate()
    {
    }

    void Scene::OnRender()
    {
        if (ImGui::Begin(GetWidgetId().c_str()))
        {
        }
        ImGui::End();
    }
} // namespace Neon::Editor::Views