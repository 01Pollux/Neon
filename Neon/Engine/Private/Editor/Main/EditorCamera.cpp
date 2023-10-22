#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Scene/EditorEntity.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Camera.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard.hpp>

#include <Input/System.hpp>

//

namespace Neon::Editor
{
    /// <summary>
    /// Get the speed of the camera with modifiers.
    /// </summary>
    static float GetCameraKeyboardSpeed()
    {
        constexpr float MinSpeed = 0.01f;
        constexpr float MaxSpeed = 50.0f;

        float Speed = Project::Config().EditorCameraKeyboardSpeed;
        if (Input::IsKeyDown(Input::EKeyboardInput::LShift))
        {
            Speed *= 2.0f;
        }
        else if (Input::IsKeyDown(Input::EKeyboardInput::LCtrl))
        {
            Speed *= 0.5f;
        }

        return std::clamp(Speed, MinSpeed, MaxSpeed);
    }

    /// <summary>
    /// Get the speed of the camera with modifiers.
    /// </summary>
    static float GetCameraMouseSpeed()
    {
        constexpr float MinSpeed = 0.01f;
        constexpr float MaxSpeed = 50.0f;

        float Speed = Project::Config().EditorCameraMouseSpeed;
        if (Input::IsKeyDown(Input::EKeyboardInput::LShift))
        {
            Speed *= 2.0f;
        }
        else if (Input::IsKeyDown(Input::EKeyboardInput::LCtrl))
        {
            Speed *= 0.5f;
        }

        return std::clamp(Speed, MinSpeed, MaxSpeed);
    }

    /// <summary>
    /// Process the camera inputs.
    /// </summary>
    static void ProcessCameraInputs(
        float DeltaTime);

    //

    void EditorEngine::AddEditorCamera()
    {
        flecs::entity Camera = this->CreateEditorEntity("EditorCamera");
        m_EditorCamera       = Camera;

        Scene::Component::Camera CameraComponent(Scene::Component::CameraType::Perspective);
        {
            CameraComponent.Viewport.FieldOfView = Project::Config().EditorCameraFOV;
            RG::CreateStandard3DRenderGraph(CameraComponent, Camera);
        }
        Camera.set(std::move(CameraComponent));

        Scene::Component::Transform TransformComponent;
        {
            TransformComponent.World.SetRotationEuler(glm::radians(Project::Config().EditorCameraRotation));
            TransformComponent.World.SetPosition(Project::Config().EditorCameraPosition);
        }
        Camera.set(std::move(TransformComponent));

        Camera.add<Scene::Editor::EditorSceneDoNotRemove>();

        Scene::EntityWorld::Get()
            .system("EditorCamera::Update")
            .kind(flecs::PreUpdate)
            .iter(
                [](flecs::iter& Iter)
                {
                    ProcessCameraInputs(Iter.delta_time());
                });
    }

    //

    enum class CameraMoveType : uint8_t
    {
        None,

        /// <summary>
        /// To Activate: Hold RMB and use WASD to move the camera.
        /// To move the camera, use WASD for forward, left, backward, and right.
        /// To move the camera up and down, use Q and E.
        /// To move the camera faster, hold Shift.
        /// To move the camera slower, hold Ctrl.
        /// To pan the camera, move the mouse.
        /// </summary>
        Fly,

        /// <summary>
        /// To Activate: Hold left alt + RMB.
        /// </summary>
        Orbit,

        /// <summary>
        /// To Activate: Hold left alt + LMB.
        /// </summary>
        Zoom
    };

    /// <summary>
    /// Determine the camera move type.
    /// </summary>
    [[nodiscard]] static CameraMoveType GetCameraMoveType()
    {
        if (Input::IsKeyDown(Input::EKeyboardInput::LAlt))
        {
            if (Input::IsMouseDown(Input::EMouseInput::Right))
            {
                return CameraMoveType::Orbit;
            }
            else if (Input::IsMouseDown(Input::EMouseInput::Left))
            {
                return CameraMoveType::Zoom;
            }
        }
        else if (Input::IsMouseDown(Input::EMouseInput::Right))
        {
            return CameraMoveType::Fly;
        }
        return CameraMoveType::None;
    }

    //

    /// <summary>
    /// Process the camera keyboard inputs.
    /// </summary>
    [[nodiscard]] static bool ProcessCameraKeyboard(
        float                           DeltaTime,
        TransformMatrix&                Transform,
        const Scene::Component::Camera& Camera)
    {
        float Speed = GetCameraKeyboardSpeed();

        Vector3 MoveDelta{};
        bool    Changed = false;

        if (Input::IsKeyDown(Input::EKeyboardInput::W))
        {
            MoveDelta += Transform.GetLookDir() * Speed;
            Changed = true;
        }
        if (Input::IsKeyDown(Input::EKeyboardInput::S))
        {
            MoveDelta -= Transform.GetLookDir() * Speed;
            Changed = true;
        }

        if (Input::IsKeyDown(Input::EKeyboardInput::A))
        {
            MoveDelta -= Transform.GetRightDir() * Speed;
            Changed = true;
        }
        if (Input::IsKeyDown(Input::EKeyboardInput::D))
        {
            MoveDelta += Transform.GetRightDir() * Speed;
            Changed = true;
        }

        if (Input::IsKeyDown(Input::EKeyboardInput::Q))
        {
            MoveDelta -= Transform.GetUpDir() * Speed;
            Changed = true;
        }
        if (Input::IsKeyDown(Input::EKeyboardInput::E))
        {
            MoveDelta += Transform.GetUpDir() * Speed;
            Changed = true;
        }

        if (Changed)
        {
            MoveDelta *= DeltaTime;
            Transform.SetPosition(Transform.GetPosition() + MoveDelta);
        }

        return Changed;
    }

    //

    /// <summary>
    /// Process the camera orbit inputs.
    /// </summary>
    bool ProcessCamera_Orbit(
        float            DeltaTime,
        TransformMatrix& Transform)
    {
        Quaternion Rotation = Transform.GetRotation();

        float Speed = GetCameraMouseSpeed();

        Vector2 MouseDelta = Input::GetMouseDelta();

        bool Changed = false;
        if (MouseDelta.x)
        {
            Transform.AppendYaw(glm::radians(MouseDelta.x * Speed * DeltaTime));
            Changed = true;
        }
        if (MouseDelta.y)
        {
            Transform.AppendPitch(glm::radians(MouseDelta.y * Speed * DeltaTime));
            Changed = true;
        }

        return Changed;
    }

    //

    /// <summary>
    /// Process the camera orbit inputs.
    /// </summary>
    bool ProcessCamera_Zoom(
        float            DeltaTime,
        TransformMatrix& Transform)
    {
        Quaternion Rotation = Transform.GetRotation();

        float Speed = GetCameraKeyboardSpeed();

        Vector2 MouseDelta = Input::GetMouseDelta();

        bool Changed = false;
        if (float Length = glm::length(MouseDelta) * (MouseDelta.x >= 0.f ? 1.f : -1.f))
        {
            Transform.SetPosition(Transform.GetPosition() + Transform.GetLookDir() * Length * Speed * DeltaTime);
            Changed = true;
        }

        return Changed;
    }

    //

    void ProcessCameraInputs(
        float DeltaTime)
    {
        auto CameraEnt = EditorEngine::Get()->GetActiveCamera();
        if (!CameraEnt || !CameraEnt.enabled()) [[unlikely]]
        {
            return;
        }

        auto& Transform = CameraEnt.get_mut<Scene::Component::Transform>()->World;
        auto  Camera    = CameraEnt.get<Scene::Component::Camera>();

        bool Changed = false;

        auto MousePos = Input::GetMousePos();

        CameraMoveType MoveType = GetCameraMoveType();
        switch (MoveType)
        {
        case CameraMoveType::None:
            return;
        case CameraMoveType::Fly:
            Changed |= ProcessCameraKeyboard(DeltaTime, Transform, *Camera);
            [[fallthrough]];
        case CameraMoveType::Orbit:
            Changed |= ProcessCamera_Orbit(DeltaTime, Transform);
            break;
        case CameraMoveType::Zoom:
            Changed |= ProcessCamera_Zoom(DeltaTime, Transform);
            break;
        default:
            break;
        }

        if (Changed)
        {
            CameraEnt.modified<Scene::Component::Transform>();
        }
    }
} // namespace Neon::Editor