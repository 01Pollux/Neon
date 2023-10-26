#include <EnginePCH.hpp>
#include <Runtime/GameLogic.hpp>

#include <Editor/Views/Types/Scene.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Scene/Component/Camera.hpp>
#include <Editor/Scene/EditorEntity.hpp>

#include <RenderGraph/RG.hpp>
#include <RHI/GlobalDescriptors.hpp>
#include <RHI/Resource/State.hpp>

#include <UI/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    SceneDisplay::SceneDisplay(
        bool IsEditorView) :
        IEditorView(IsEditorView ? StandardViews::s_SceneViewWidgetId : StandardViews::s_GameViewWidgetId),
        m_IsEditorView(IsEditorView)
    {
    }

    void SceneDisplay::OnUpdate()
    {
        auto Window = ImGui::FindWindowByName(GetWidgetId().c_str());
        if (!Window) [[unlikely]]
        {
            return;
        }

        // If we are in editor scene view, disable/enable camera based on window state.
        flecs::entity Camera = GetCurrentCamera();
        if (!Camera) [[unlikely]]
        {
            return;
        }

        // Check if window is active and not collapsed to enable/disable camera.
        if (Window->Hidden || Window->Collapsed) [[unlikely]]
        {
            return;
        }

        auto CameraData = Camera.get_mut<Scene::Component::Camera>();
        if (!CameraData) [[unlikely]]
        {
            return;
        }

        auto RenderGraph = CameraData->GetRenderGraph();
        if (!RenderGraph) [[unlikely]]
        {
            return;
        }

        auto& GraphStorage = RenderGraph->GetStorage();

        Size2 Size(CameraData->Viewport.Width, CameraData->Viewport.Height);
        bool  Changed       = false;
        bool  HasClientSize = false;

        if (CameraData->Viewport.ClientWidth)
        {
            HasClientSize = true;
            Size.x        = float(Window->Size.x);
            if (Size.x != CameraData->Viewport.Width)
            {
                Changed = true;
            }
        }
        if (CameraData->Viewport.ClientHeight)
        {
            HasClientSize = true;
            Size.y        = float(Window->Size.y);
            if (Size.y != CameraData->Viewport.Height)
            {
                Changed = true;
            }
        }

        if (Changed)
        {
            CameraData->Viewport.Width  = Size.x;
            CameraData->Viewport.Height = Size.y;
            GraphStorage.SetOutputImageSize(HasClientSize ? Size : std::optional<Size2I>{});

            Camera.modified<Scene::Component::Camera>();
        }
    }

    void SceneDisplay::OnRender()
    {
        flecs::entity Camera = GetCurrentCamera();

        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

        // Only for editor's camera, adding/removing Scene::Editor::SceneCameraCanMove depending on mouse and window state
        if (m_IsEditorView)
        {
            if (Window && ImGui::IsWindowHovered())
            {
                Camera.add<Scene::Editor::SceneCameraCanMove>();
            }
            else
            {
                Camera.remove<Scene::Editor::SceneCameraCanMove>();
            }
        }

        if (!Window)
        {
            return;
        }

        if (!Camera || !Camera.enabled()) [[unlikely]]
        {
            return;
        }

        auto CameraData = Camera.get<Scene::Component::Camera>();
        if (!CameraData) [[unlikely]]
        {
            return;
        }

        auto RenderGraph = CameraData->GetRenderGraph();
        if (!RenderGraph) [[unlikely]]
        {
            return;
        }

        auto& GraphStorage = RenderGraph->GetStorage();
        if (GraphStorage.IsEmpty()) [[unlikely]]
        {
            return;
        }

        auto& FinalImage = GraphStorage.GetOutputImage().Get();

        // Transition FinalImage to SRV.
        RHI::IResourceStateManager::Get()->TransitionResource(
            FinalImage.get(),
            RHI::EResourceState::PixelShaderResource);

        RHI::IResourceStateManager::Get()->FlushBarriers();

        auto StagedDescriptor = RHI::IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        auto FinalImageSrv    = StagedDescriptor->Allocate(1);

        FinalImageSrv.Heap->CreateShaderResourceView(
            FinalImageSrv.Offset,
            FinalImage.get());

        ImGui::Image(
            ImTextureID(FinalImageSrv.GetCpuHandle().Value),
            ImGui::GetContentRegionAvail());
    }

    flecs::entity SceneDisplay::GetCurrentCamera()
    {
        flecs::entity Camera;
        if (m_IsEditorView)
        {
            Camera = EditorEngine::Get()->GetEditorCamera();
        }
        else
        {
            auto FirstCameraFilter =
                Scene::EntityWorld::Get()
                    .filter_builder<const Scene::Component::Camera>()
                    .with<Scene::Component::ActiveSceneEntity>()
                    .in()
                    .build();

            int MaxPriority = std::numeric_limits<int>::min();
            FirstCameraFilter.each(
                [&Camera, &MaxPriority](flecs::entity Entity, const Scene::Component::Camera& CameraData)
                {
                    if (Entity == EditorEngine::Get()->GetEditorCamera())
                    {
                        return;
                    }

                    if (MaxPriority < CameraData.RenderPriority)
                    {
                        Camera      = Entity;
                        MaxPriority = CameraData.RenderPriority;
                    }
                });
        }
        return Camera;
    }
} // namespace Neon::Editor::Views