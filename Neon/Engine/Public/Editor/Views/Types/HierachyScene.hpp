#pragma once

#include <Editor/Views/View.hpp>
#include <Scene/EntityWorld.hpp>
#include <functional>

namespace Neon::Editor::Views
{
    class SceneHierachy : public IEditorView
    {
    public:
        SceneHierachy();

        void OnRender() override;

    private:
        /// <summary>
        /// Display a scene object in the hierachy view.
        /// </summary>
        void DispalySceneObject(
            Scene::EntityHandle              EntHandle,
            std::move_only_function<void()>& DeferredTask,
            bool                             Editable = true);

        /// <summary>
        /// Display Editor's entities in the scene
        /// </summary>
        void DisplayEditorEntities();

        /// <summary>
        /// Helper function for displaying popup context on entities
        /// </summary>
        void DisplayEntityPopup(
            const flecs::entity&             ParentEntHandle,
            std::move_only_function<void()>& DeferredTask);

    private:
        char          m_RenameBuffer[255]{};
        bool          m_DisplayEditorEntities = false;
        flecs::entity m_EntityToRename;
    };
} // namespace Neon::Editor::Views