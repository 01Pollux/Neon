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
        if (m_IsEditorView)
        {
            flecs::entity Camera = EditorEngine::Get()->GetActiveCamera();
            if (!Camera) [[unlikely]]
            {
                return;
            }

            // Check if window is active and not collapsed to enable/disable camera.
            if (Window->Hidden || Window->Collapsed) [[unlikely]]
            {
                if (Camera.enabled())
                {
                    Camera.disable();
                }
                return;
            }
            else
            {
                if (!Camera.enabled())
                {
                    Camera.enable();
                }
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
                RenderGraph->GetStorage().SetOutputImageSize(HasClientSize ? Size : std::optional<Size2I>{});

                Camera.modified<Scene::Component::Camera>();
            }
        }
    }

    void SceneDisplay::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        // TODO: option to switch camera.
        flecs::entity Camera = EditorEngine::Get()->GetActiveCamera();
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

        auto& FinalImage = RenderGraph->GetStorage().GetOutputImage().Get();

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
} // namespace Neon::Editor::Views