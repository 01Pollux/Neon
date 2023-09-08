#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

#include <Input/System.hpp>

namespace Neon::Editor
{
    /// <summary>
    /// Get the speed of the camera with modifiers.
    /// </summary>
    static float GetCameraSpeed()
    {
        constexpr float MinSpeed = 0.01f;
        constexpr float MaxSpeed = 50.0f;

        float Speed = Project::Config().EditorCameraSpeed;
        if (Input::IsKeyDown(Input::EKeyboardInput::LShift))
        {
            Speed *= 2.0f;
        }
        else if (Input::IsKeyDown(Input::EKeyboardInput::LCtrl))
        {
            Speed *= 0.5f;
        }
        else if (Input::IsKeyDown(Input::EKeyboardInput::LAlt))
        {
            Speed *= 0.1f;
        }

        return std::clamp(Speed, MinSpeed, MaxSpeed);
    }

    void EditorEngine::AddEditorCamera()
    {
        flecs::entity Camera = this->CreateEditorEntity("EditorCamera");
        m_EditorCamera       = Camera;

        Scene::Component::Camera CameraComponent(Scene::Component::CameraType::Perspective);
        {
            CameraComponent.Viewport.FieldOfView = Project::Config().EditorCameraFOV;
            RG::CreateStandard2DRenderGraph(CameraComponent, Camera);
        }
        Camera.set(std::move(CameraComponent));

        Scene::Component::Transform TransformComponent;
        {
            TransformComponent.World.SetRotationEuler(glm::radians(Project::Config().EditorCameraRotation));
            TransformComponent.World.SetPosition(Project::Config().EditorCameraPosition);
        }
        Camera.set(std::move(TransformComponent));

        Camera.add<Scene::Editor::EditorSceneDoNotRemove>();

        auto UpdateCamera =
            Scene::EntityWorld::Get()
                .system("EditorCamera::Update")
                .kind(flecs::PreUpdate)
                .iter(
                    [](flecs::iter& Iter)
                    {
                        auto CameraEnt = EditorEngine::Get()->GetActiveCamera();
                        if (!CameraEnt || !CameraEnt.enabled()) [[unlikely]]
                        {
                            return;
                        }

                        auto& Transform = CameraEnt.get_mut<Scene::Component::Transform>()->World;
                        auto  Camera    = CameraEnt.get<Scene::Component::Camera>();

                        float   Speed = GetCameraSpeed();
                        Vector3 MoveDelta{};
                        bool    Changed = false;

                        if (Input::IsKeyDown(Input::EKeyboardInput::W))
                        {
                            MoveDelta += Camera->Forward * Speed;
                            Changed = true;
                        }
                        if (Input::IsKeyDown(Input::EKeyboardInput::S))
                        {
                            MoveDelta -= Camera->Forward * Speed;
                            Changed = true;
                        }

                        if (Input::IsKeyDown(Input::EKeyboardInput::A))
                        {
                            MoveDelta -= Camera->Right * Speed;
                            Changed = true;
                        }
                        if (Input::IsKeyDown(Input::EKeyboardInput::D))
                        {
                            MoveDelta += Camera->Right * Speed;
                            Changed = true;
                        }

                        if (Input::IsKeyDown(Input::EKeyboardInput::Q))
                        {
                            MoveDelta -= Camera->Up * Speed;
                            Changed = true;
                        }
                        if (Input::IsKeyDown(Input::EKeyboardInput::E))
                        {
                            MoveDelta += Camera->Up * Speed;
                            Changed = true;
                        }

                        if (Changed)
                        {
                            MoveDelta *= Iter.delta_time();
                            Transform.SetPosition(Transform.GetPosition() + MoveDelta);
                            CameraEnt.modified<Scene::Component::Transform>();
                        }
                    });
    }
} // namespace Neon::Editor