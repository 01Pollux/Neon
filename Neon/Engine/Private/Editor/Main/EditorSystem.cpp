#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Editor/Views/Components/EngineComponents.hpp>
#include <Editor/Scene/EditorEntity.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

namespace Neon::Editor
{
    void EditorEngine::AddStandardEditorSystem()
    {
        // Register editor world components
        RegisterEditorWorldComponents();

        // Register component handlers
        AddStandardComponentHandlers();

        // Register editor's main camera
        AddEditorCamera();

        //

        // This is test: TODO remove
        flecs::world World = EditorEngine::Get()->GetLogic()->GetEntityWorld();
        auto         Root  = EditorEngine::Get()->GetEditorRootEntity();

        auto A = World.entity("Unnamed Entity");
        auto B = World.entity("B");
        auto C = World.entity("C");
        auto D = World.entity("D");
        auto E = World.entity("E");
        auto F = World.entity("F");
        auto G = World.entity("G");

        A.child_of(Root);
        {
            B.child_of(A);
            C.child_of(A);
            {
                D.child_of(C);
            }
            E.child_of(A);
        }

        Scene::Component::Transform Tr;
        Tr.World.SetPosition({ 30.0f, 20.0f, 10.0f });
        Tr.World.SetRotationEuler(glm::radians(Vec::Forward<Vector3> * -90.f));
        F.set(Tr);

        A.add<Scene::Editor::HideInEditor>();
        F.child_of(Root);
        G.child_of(Root);
    }

    //

    void EditorEngine::RegisterEditorWorldComponents()
    {
        flecs::world World = GetLogic()->GetEntityWorld();
        m_EditorRootEntity = World.entity("_EditorRoot");

        NEON_REGISTER_FLECS(Scene::Editor::HideInEditor);
        NEON_REGISTER_FLECS(Scene::Editor::SelectedForEditor);
        NEON_REGISTER_FLECS(Scene::Editor::WorldEditorMode);

        World.add<Scene::Editor::WorldEditorMode>();
    }

    //

    void EditorEngine::AddStandardComponentHandlers()
    {
        RegisterStandardComponentHandler<TransformComponentHandler, Scene::Component::Transform>();
    }

    //

    void EditorEngine::AddEditorCamera()
    {
        flecs::world World = GetLogic()->GetEntityWorld();

        auto RootEntity = EditorEngine::Get()->GetEditorRootEntity();
        auto Camera     = World.entity("_EditorCamera").child_of(RootEntity);

        Scene::Component::Camera CameraComponent(Scene::Component::CameraType::Orthographic);
        {
            RG::CreateStandard2DRenderGraph(CameraComponent, Camera);

            CameraComponent.Viewport.OrthographicSize = 50.0f;
            CameraComponent.Viewport.NearPlane        = -1.0f;
            CameraComponent.Viewport.FarPlane         = 10.0f;
        }
        Camera.set(std::move(CameraComponent));

        Scene::Component::Transform TransformComponent;
        {
            TransformComponent.World.SetRotationEuler(glm::radians(Vec::Right<Vector3> * -90.f));
            TransformComponent.World.SetPosition(Vec::Backward<Vector3> * 10.f);
        }
        Camera.set(std::move(TransformComponent));

        RootEntity.add<Scene::Component::MainCamera>(Camera);
    }

    //

    flecs::entity EditorEngine::GetEditorRootEntity() const
    {
        flecs::world World = GetLogic()->GetEntityWorld();
        return flecs::entity(World, m_EditorRootEntity);
    }

    flecs::entity EditorEngine::GetEditorActiveRootEntity() const
    {
        auto World = GetLogic()->GetEntityWorld();
        return World.GetWorld().has<Scene::Editor::WorldEditorMode>() ? GetEditorRootEntity() : World.GetRootEntity();
    }
} // namespace Neon::Editor