#pragma once

#include <Runtime/GameEngine.hpp>
#include <Editor/Views/View.hpp>
#include <Editor/Views/Component.hpp>
#include <Scene/RuntimeScene.hpp>

#include <UI/Fonts/FontAwesome5.hpp>

#include <unordered_map>
#include <unordered_set>

namespace Neon::Asset
{
    class IAssetPackage;
} // namespace Neon::Asset

namespace Neon::Editor
{
    class EditorEngine : public Runtime::GameEngine
    {
    public:
        /// <summary>
        /// Get the singleton instance of the editor.
        /// </summary>
        [[nodiscard]] static EditorEngine* Get()
        {
            return static_cast<EditorEngine*>(GameEngine::Get());
        }

        void Initialize(
            Config::EngineConfig Config) override;

        void PreUpdate() override;

        void PostRender() override;

    public:
        /// <summary>
        /// Register a new editor view.
        /// </summary>
        [[nodiscard]] void RegisterView(
            const StringU8&   Name,
            UPtr<IEditorView> View,
            bool              InitialVisible = false);

        /// <summary>
        /// Register a new editor view.
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::derived_from<_Ty, IEditorView>
        [[nodiscard]] void RegisterView(
            const StringU8& Name,
            bool            InitialVisible = false,
            _Args&&... Args)
        {
            RegisterView(Name, std::make_unique<_Ty>(std::forward<_Args>(Args)...), InitialVisible);
        }

        /// <summary>
        /// Unregister an editor view.
        /// </summary>
        void UnregisterView(
            const StringU8& Name);

        /// <summary>
        /// Open the editor view.
        /// </summary>
        void OpenView(
            const StringU8& Name);

        /// <summary>
        /// Check if the editor view is open.
        /// </summary>
        bool IsViewOpen(
            const StringU8& Name);

        /// <summary>
        /// Close the editor view.
        /// </summary>
        void CloseView(
            const StringU8& Name);

    public:
        /// <summary>
        /// Register the component handler.
        /// </summary>
        void RegisterComponentHandler(
            const flecs::id&         ComponentId,
            IEditorComponentHandler* Handler);

        /// <summary>
        /// Unregister the component handler.
        /// </summary>
        void UnregisterComponentHandler(
            const flecs::id&         ComponentId,
            IEditorComponentHandler* Handler);

        /// <summary>
        /// Unregister the component handler.
        /// </summary>
        void UnregisterComponentHandler(
            IEditorComponentHandler* Handler);

        /// <summary>
        /// Dispatch the component handlers.
        /// </summary>
        void DispatchComponentHandlers(
            Scene::EntityHandle EntHandle,
            const flecs::id&    ComponentId);

    private:
        /// <summary>
        /// Register the editor world components, camera.
        /// </summary>
        void AddStandardEditorSystem();

        /// <summary>
        /// Register the editor world components.
        /// </summary>
        void RegisterEditorWorldComponents();

        /// <summary>
        /// Add the standard component handlers.
        /// </summary>
        void AddStandardComponentHandlers();

        /// <summary>
        /// Add the editor camera.
        /// </summary>
        void AddEditorCamera();

    private:
        /// <summary>
        /// Add the standard views to the editor.
        /// </summary>
        void AddStandardViews();

        /// <summary>
        /// Begin the editor space by initializing dockspace.
        /// </summary>
        bool BeginEditorSpace();

        /// <summary>
        /// Render the menu bar.
        /// </summary>
        void RenderMenuBar();

        /// <summary>
        /// End the editor space by finalizing dockspace.
        /// </summary>
        void EndEditorSpace();

    public:
        /// <summary>
        /// Get the editor active root entity.
        /// </summary>
        [[nodiscard]] const Scene::RuntimeScene& GetRuntimeScene() const;

        /// <summary>
        /// Get the editor active root entity.
        /// </summary>
        [[nodiscard]] const Scene::RuntimeScene& GetEditorScene() const;

        /// <summary>
        /// Get either the editor camera or the main camera.
        /// </summary>
        [[nodiscard]] Scene::EntityHandle GetMainCamera(
            bool EditorCamera) const;

        /// <summary>
        /// Check if the editor is in editor mode.
        /// </summary>
        [[nodiscard]] bool IsInEditorMode() const;

    public:
        /// <summary>
        /// Get the content package.
        /// </summary>
        [[nodiscard]] Asset::IAssetPackage* GetContentPackage() const
        {
            return m_ContentPackage;
        }

    private:
        /// <summary>
        /// Helper function to register the standard component handler.
        /// </summary>
        template<typename _Ty, typename... _Args>
        void RegisterStandardComponentHandler()
        {
            auto Handler    = std::make_unique<_Ty>();
            auto HandlerPtr = Handler.get();
            m_StandardComponentHandlers.emplace(std::move(Handler));

            flecs::world World = Scene::EntityWorld::Get();
            for (auto ComponentId : { World.component<_Args>()... })
            {
                RegisterComponentHandler(ComponentId, HandlerPtr);
            }
        }

    private:
        std::unordered_map<StringU8, UPtr<IEditorView>> m_Views;

        std::set<UPtr<IEditorComponentHandler>> m_StandardComponentHandlers;

        std::unordered_map<uint64_t, std::vector<IEditorComponentHandler*>> m_ComponentHandlers;

        std::unordered_set<IEditorView*> m_OpenViews;

        /// <summary>
        /// TODO: move it to ProjectInstance.
        /// </summary>
        Asset::IAssetPackage* m_ContentPackage;

        Scene::RuntimeScene m_RuntimeScene;
        Scene::RuntimeScene m_EditorScene;

        bool m_IsTitlebarHovered = false;
    };
} // namespace Neon::Editor