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
            std::move_only_function<void()>& DeferredTask);

    private:
        char          m_RenameBuffer[256];
        flecs::entity m_EntityToRename;
    };
} // namespace Neon::Editor::Views