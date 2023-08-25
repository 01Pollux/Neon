#include <EnginePCH.hpp>
#include <Editor/Views/Types/Scene.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Runtime/GameLogic.hpp>

#include <Scene/Component/Camera.hpp>
#include <RenderGraph/RG.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <UI/imcxx/all_in_one.hpp>

namespace Neon::Editor::Views
{
    SceneDisplay::SceneDisplay() :
        IEditorView(StandardViews::s_SceneViewWidgetId)
    {
    }

    void SceneDisplay::OnUpdate()
    {
        auto Window = ImGui::FindWindowByName(GetWidgetId().c_str());
        if (!Window) [[unlikely]]
        {
            return;
        }

        auto RootEntity = EditorEngine::Get()->GetEditorRootEntity();
        auto Camera     = RootEntity.target<Scene::Component::MainCamera>();
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
        }
        else
        {
            if (!Camera.enabled())
            {
                Camera.enable();
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

        auto RootEntity = EditorEngine::Get()->GetEditorRootEntity();
        auto Camera     = RootEntity.target<Scene::Component::MainCamera>();
        if (!Camera) [[unlikely]]
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

        auto& FinalImage      = RenderGraph->GetStorage().GetOutputImage().Get();
        auto  FrameDescriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        auto  FinalImageSrv   = FrameDescriptor->Allocate(1);

        FinalImageSrv.Heap->CreateShaderResourceView(
            FinalImageSrv.Offset,
            FinalImage.get());

        ImGui::Image(
            ImTextureID(FinalImageSrv.GetCpuHandle().Value),
            ImGui::GetContentRegionAvail() /*,
             ImVec2(0, 1),
             ImVec2(1, 0)*/
        );
    }
} // namespace Neon::Editor::Views