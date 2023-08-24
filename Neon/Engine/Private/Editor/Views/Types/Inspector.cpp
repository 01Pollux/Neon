#include <EnginePCH.hpp>
#include <Editor/Views/Types/Inspector.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>
#include <Runtime/GameLogic.hpp>

#include <UI/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    Inspector::Inspector() :
        IEditorView(StandardViews::s_InspectorViewWidgetId)
    {
    }

    void Inspector::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        flecs::world World = EditorEngine::Get()->GetLogic()->GetEntityWorld();

        auto SelectedEntity = World.target<Scene::Editor::SelectedForEditor>();
        if (!SelectedEntity)
        {
            return;
        }

        SelectedEntity.each(
            [&SelectedEntity](flecs::id ComponentId)
            {
                EditorEngine::Get()->DispatchComponentHandlers(SelectedEntity, ComponentId);
            });
    }
} // namespace Neon::Editor::Views