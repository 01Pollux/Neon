#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Editor/Scene/EditorEntity.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

namespace Neon::Editor
{
    void EditorEngine::AddEditorCamera()
    {
        flecs::entity Camera = this->CreateEditorEntity("_EditorCamera_");
        m_EditorCamera       = Camera;

        Scene::Component::Camera CameraComponent(Scene::Component::CameraType::Perspective);
        {
            RG::CreateStandard2DRenderGraph(CameraComponent, Camera);
        }
        Camera.set(std::move(CameraComponent));

        Scene::Component::Transform TransformComponent;
        {
            TransformComponent.World.SetRotationEuler(glm::radians(Vec::Right<Vector3> * -90.f));
            TransformComponent.World.SetPosition(Vec::Backward<Vector3> * 10.f);
        }
        Camera.set(std::move(TransformComponent));

        Camera.add<Scene::Editor::EditorSceneDoNotRemove>();
    }

} // namespace Neon::Editor