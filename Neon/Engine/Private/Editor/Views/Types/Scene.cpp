#include <EnginePCH.hpp>
#include <Editor/Views/Types/Scene.hpp>

#include <UI/imcxx/all_in_one.hpp>

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
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }
    }
} // namespace Neon::Editor::Views