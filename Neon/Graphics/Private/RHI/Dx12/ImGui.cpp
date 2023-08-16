#include <GraphicsPCH.hpp>
#include <RHI/ImGui.hpp>

#include <Window/Window.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>

#include <ImGui/imgui.h>
#include <ImGui/backends/imgui_impl_dx12.h>
#include <ImGui/backends/imgui_impl_glfw.h>

extern ImGui_ImplDX12_Data* ImGui_ImplDX12_GetBackendData();

namespace Neon::RHI::ImGuiRHI
{
    static std::vector<ImDrawCmd*>                  s_DrawCommands;
    static std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> s_DescriptorHandles;
    static std::map<SIZE_T, uint32_t>               s_DescriptorHandlesRemapTexID;
    static std::vector<UINT>                        s_DescriptorHandlesSizes;

    void InitializeImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& IO = ImGui::GetIO();
        IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        auto Window       = ISwapchain::Get()->GetWindow();
        auto WindowHandle = Window->GetHandle();
        auto Swapchain    = RHI::ISwapchain::Get();

        ImGui_ImplGlfw_InitForOther(WindowHandle, true);
        // Setting descriptor heap and font heaps to null since we are using our own allocator.
        ImGui_ImplDX12_Init(
            RHI::Dx12RenderDevice::Get()->GetDevice(),
            UINT(Swapchain->GetBackbufferCount()),
            RHI::CastFormat(Swapchain->GetFormat()),
            nullptr,
            {},
            {});
    }

    void ShutdownImGui()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void BeginImGuiFrame()
    {
        auto BackendData = ImGui_ImplDX12_GetBackendData();

        auto StagedDescriptor = IStagedDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
        auto FrameDescriptor  = static_cast<Dx12FrameDescriptorHeap*>(IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView));
        auto Handle           = StagedDescriptor->Allocate(1);

        BackendData->pd3dSrvDescHeap       = FrameDescriptor->GetHeap();
        BackendData->hFontSrvCpuDescHandle = { Handle.GetCpuHandle().Value };
        BackendData->hFontSrvGpuDescHandle = { Handle.GetCpuHandle().Value };

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EndImGuiFrame()
    {
        ImGui::Render();

        RHI::GraphicsCommandContext GraphicsContext;

        auto Swapchain    = RHI::ISwapchain::Get();
        auto Window       = Swapchain->GetWindow();
        auto StateManager = RHI::IResourceStateManager::Get();
        auto BackBuffer   = Swapchain->GetBackBuffer();
        auto CommandList  = GraphicsContext.Append();

        auto GlobalDrawData = ImGui::GetDrawData();

        auto& Viewports = ImGui::GetPlatformIO().Viewports;
        // Iterate all viewports, and collect all draw commands' texture descriptors.
        uint32_t DescriptorOffset = 0;
        for (auto Viewport : Viewports)
        {
            auto DrawData = Viewport->DrawData;
            for (int i = 0; i < DrawData->CmdListsCount; i++)
            {
                auto DrawList = DrawData->CmdLists[i];
                for (int CmdId = 0; CmdId < DrawList->CmdBuffer.Size; CmdId++)
                {
                    auto DrawCmd = &DrawList->CmdBuffer[CmdId];
                    if (!DrawCmd->UserCallback && DrawCmd->TextureId)
                    {
                        auto Handle                = std::bit_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(DrawCmd->TextureId);
                        auto [Iter, NewlyInserted] = s_DescriptorHandlesRemapTexID.emplace(Handle.ptr, DescriptorOffset);

                        if (NewlyInserted)
                        {
                            s_DescriptorHandles.emplace_back(Handle);
                            DescriptorOffset++;
                        }

                        s_DrawCommands.emplace_back(DrawCmd);
                    }
                }
            }
        }

        // Copy the texture descriptors to the frame descriptor heap.
        if (!s_DrawCommands.empty()) [[unlikely]]
        {
            auto FrameDescriptor = IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView);
            auto DstDescriptor   = FrameDescriptor->Allocate(uint32_t(s_DescriptorHandles.size()));

            auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

            for (size_t i = s_DescriptorHandlesSizes.size(); i < s_DescriptorHandles.size(); i++)
            {
                s_DescriptorHandlesSizes.emplace_back(1);
            }

            for (auto& DrawCmd : s_DrawCommands)
            {
                auto Handle = std::bit_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(DrawCmd->TextureId);
                auto Offset = s_DescriptorHandlesRemapTexID.at(Handle.ptr);

                DrawCmd->TextureId = std::bit_cast<ImTextureID>(DstDescriptor.GetGpuHandle(Offset).Value);
            }

            D3D12_CPU_DESCRIPTOR_HANDLE DxDescriptors[]      = { DstDescriptor.GetCpuHandle().Value };
            UINT                        DxDescriptorsSizes[] = { UINT(s_DescriptorHandles.size()) };

            Dx12Device->CopyDescriptors(
                1,
                DxDescriptors,
                DxDescriptorsSizes,
                UINT(s_DescriptorHandles.size()),
                s_DescriptorHandles.data(),
                s_DescriptorHandlesSizes.data(),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            s_DescriptorHandles.clear();
            s_DescriptorHandlesRemapTexID.clear();
            s_DrawCommands.clear();
        }

        // Set viewport, scissor rect and render target view
        {
            auto& Size = RHI::ISwapchain::Get()->GetSize();

            CommandList->SetViewport(
                ViewportF{
                    .Width    = float(Size.Width()),
                    .Height   = float(Size.Height()),
                    .MaxDepth = 1.f,
                });
            CommandList->SetScissorRect(RectF(Vec::Zero<Vector2>, Size));

            auto View = RHI::ISwapchain::Get()->GetBackBufferView();

            CommandList->SetRenderTargets(
                View, 1);
        }

        auto Dx12CmdList = dynamic_cast<Dx12CommandList*>(CommandList)->Get();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Dx12CmdList);

        // Transition the backbuffer to a present state.
        StateManager->TransitionResource(
            BackBuffer,
            RHI::MResourceState_Present);

        StateManager->FlushBarriers(CommandList);

        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault(nullptr, Dx12CmdList);
        }
    }

    //

    void SetDefaultTheme()
    {
        ImGuiStyle& Style = ImGui::GetStyle();

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            Style.WindowRounding              = 0.0f;
            Style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        Style.Alpha                     = 1.0f;
        Style.DisabledAlpha             = 0.6000000238418579f;
        Style.WindowPadding             = ImVec2(8.0f, 8.0f);
        Style.WindowRounding            = 4.0f;
        Style.WindowBorderSize          = 0.0f;
        Style.WindowMinSize             = ImVec2(32.0f, 32.0f);
        Style.WindowTitleAlign          = ImVec2(0.0f, 0.5f);
        Style.WindowMenuButtonPosition  = ImGuiDir_Left;
        Style.ChildRounding             = 0.0f;
        Style.ChildBorderSize           = 1.0f;
        Style.PopupRounding             = 4.0f;
        Style.PopupBorderSize           = 1.0f;
        Style.FramePadding              = ImVec2(4.0f, 3.0f);
        Style.FrameRounding             = 2.5f;
        Style.FrameBorderSize           = 0.0f;
        Style.ItemSpacing               = ImVec2(8.0f, 4.0f);
        Style.ItemInnerSpacing          = ImVec2(4.0f, 4.0f);
        Style.CellPadding               = ImVec2(4.0f, 2.0f);
        Style.IndentSpacing             = 21.0f;
        Style.ColumnsMinSpacing         = 6.0f;
        Style.ScrollbarSize             = 11.0f;
        Style.ScrollbarRounding         = 2.5f;
        Style.GrabMinSize               = 10.0f;
        Style.GrabRounding              = 2.0f;
        Style.TabRounding               = 3.5f;
        Style.TabBorderSize             = 0.0f;
        Style.TabMinWidthForCloseButton = 0.0f;
        Style.ColorButtonPosition       = ImGuiDir_Right;
        Style.ButtonTextAlign           = ImVec2(0.5f, 0.5f);
        Style.SelectableTextAlign       = ImVec2(0.0f, 0.0f);

        Style.Colors[ImGuiCol_Text]                  = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        Style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, 1.0f);
        Style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_Border]                = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        Style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        Style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        Style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        Style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        Style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
        Style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
        Style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
        Style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_Button]                = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        Style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_Header]                = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        Style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_Separator]             = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        Style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        Style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        Style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        Style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
        Style.Colors[ImGuiCol_Tab]                   = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_TabHovered]            = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_TabActive]             = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        Style.Colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
        Style.Colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
        Style.Colors[ImGuiCol_TableBorderLight]      = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
        Style.Colors[ImGuiCol_TableRowBg]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        Style.Colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
        Style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        Style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        Style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
        Style.Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
        Style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    }
} // namespace Neon::RHI::ImGuiRHI