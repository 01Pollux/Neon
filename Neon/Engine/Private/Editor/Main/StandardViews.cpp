#include <EnginePCH.hpp>
#include <Window/Window.hpp>

#include <Editor/Main/EditorEngine.hpp>
#include <Editor/Views/Types/ContentBrowser.hpp>
#include <Editor/Views/Types/Console.hpp>
#include <Editor/Views/Types/Game.hpp>
#include <Editor/Views/Types/HierachyScene.hpp>
#include <Editor/Views/Types/Inspector.hpp>
#include <Editor/Views/Types/Scene.hpp>

#include <UI/WindowUtils.hpp>

namespace Neon::Editor
{
    void EditorEngine::AddStandardViews()
    {
        RegisterView<Views::Console>("_Console", true);
        RegisterView<Views::ContentBrowser>("_ContentBrowser", true);
        RegisterView<Views::GameDisplay>("_GameDisplay", true);
        RegisterView<Views::SceneHierachy>("_Hierachy", true);
        RegisterView<Views::Inspector>("_Inspector", true);
        RegisterView<Views::SceneDisplay>("_SceneDisplay", true);
    }

    bool EditorEngine::BeginEditorSpace()
    {
        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        constexpr ImGuiWindowFlags EditorWindowFlags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* Viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(Viewport->WorkPos);
        ImGui::SetNextWindowSize(Viewport->WorkSize);
        ImGui::SetNextWindowViewport(Viewport->ID);

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

        bool EditorOpen;
        bool IsMaximized = GetWindow()->IsMaximized();
        {
            imcxx::shared_style OverrideStyle(
                ImGuiStyleVar_WindowPadding, IsMaximized ? ImVec2{ 6.0f, 6.0f } : ImVec2{ 1.0f, 1.0f },
                ImGuiStyleVar_WindowBorderSize, 3.0f);
            imcxx::shared_color OverrideBg(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });

            EditorOpen = ImGui::Begin("Neon Editor", nullptr, EditorWindowFlags);
        }

        {
            imcxx::shared_color BorderColor(ImGuiCol_Border, IM_COL32(50, 50, 50, 255));
            // Draw window border if the window is not maximized
            if (!IsMaximized)
                UI::WindowUtils::RenderWindowOuterBorders(ImGui::GetCurrentWindow());
        }

        // Submit the DockSpace
        ImGuiID DockerspaceId = ImGui::GetID("MainDockspace##NEON");
        if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) &&
            !ImGui::DockBuilderGetNode(DockerspaceId))
        {
            // Clear out existing layout
            ImGui::DockBuilderRemoveNode(DockerspaceId);

            // Add empty node
            ImGui::DockBuilderAddNode(DockerspaceId, ImGuiDockNodeFlags_DockSpace);

            // Main node should cover entire window
            ImGui::DockBuilderSetNodeSize(DockerspaceId, ImGui::GetWindowViewport()->Size);

            // Build dock layout
            ImGuiID Center = DockerspaceId;

            ImGuiID TopCenter = ImGui::DockBuilderSplitNode(Center, ImGuiDir_Up, .45f, nullptr, &Center);
            ImGuiID Bottom    = Center;

            ImGuiID Left  = ImGui::DockBuilderSplitNode(TopCenter, ImGuiDir_Left, .25f, nullptr, &TopCenter);
            ImGuiID Right = ImGui::DockBuilderSplitNode(TopCenter, ImGuiDir_Right, .25f, nullptr, &TopCenter);

            /**
             *  We want to create a dock space like this:
             * __________________________________
             * |       |_B_|_C_|_______|        |
             * |       |               |        |
             * |   A   |               |   D    |
             * |       |      B/C      |        |
             * |       |               |        |
             * |_______|_______________|________|
             * |_E_|_F_|________________________|
             * |                                |
             * |                E/F             |
             * |________________________________|
             *
             * A = Hierarchy view
             * B = Scene view
             * C = Game view
             * D = Inspector view
             * E = Console view
             * F = Content browser
             */

            ImGui::DockBuilderDockWindow(StandardViews::s_HierachyViewWidgetId, Left);
            ImGui::DockBuilderDockWindow(StandardViews::s_SceneViewWidgetId, TopCenter);
            ImGui::DockBuilderDockWindow(StandardViews::s_GameViewWidgetId, TopCenter);
            ImGui::DockBuilderDockWindow(StandardViews::s_InspectorViewWidgetId, Right);
            ImGui::DockBuilderDockWindow(StandardViews::s_ConsoleViewWidgetId, Bottom);
            ImGui::DockBuilderDockWindow(StandardViews::s_ContentBrowserWidgetId, Bottom);

            ImGui::DockBuilderFinish(DockerspaceId);
        }

        constexpr ImGuiDockNodeFlags MainEditorWindowDockSpaceFlags =
            ImGuiDockNodeFlags_None |
            ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::DockSpace(DockerspaceId, {}, MainEditorWindowDockSpaceFlags);
        return EditorOpen;
    }

    void EditorEngine::RenderMenuBar()
    {
        bool DisplayMenuBar;
        {
            ImVec2 FramePadding = ImGui::GetStyle().FramePadding;
            FramePadding.y *= 2.8f;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, FramePadding);
            DisplayMenuBar = ImGui::BeginMainMenuBar();
        }

        m_IsTitlebarHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        if (!DisplayMenuBar)
        {
            ImGui::PopStyleVar();
            return;
        }

        auto CurPos = ImGui::GetCursorPos();
        // Render the titlebar buttons
        {
            auto TitlebarHeight = ImGui::GetCurrentWindow()->MenuBarHeight();
            auto ButtonSize     = ImVec2(TitlebarHeight * 1.4f, TitlebarHeight);

            imcxx::shared_style OverrideSpacing(ImGuiStyleVar_ItemSpacing, ImVec2{});
            imcxx::shared_color OverrideColor(
                ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg),
                ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_ScrollbarGrabActive),
                ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_ScrollbarGrabHovered));

            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ButtonSize.x * 3);

            if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE, ButtonSize))
            {
                GetWindow()->Minimize();
            }

            bool IsMaximized = GetWindow()->IsMaximized();
            if (ImGui::Button(IsMaximized ? ICON_FA_WINDOW_RESTORE : ICON_FA_WINDOW_MAXIMIZE, ButtonSize))
            {
                if (IsMaximized)
                {
                    GetWindow()->Restore();
                }
                else
                {
                    GetWindow()->Maximize();
                }
            }

            if (ImGui::Button(ICON_FA_TIMES, ButtonSize))
            {
                GetWindow()->Close();
            }
        }
        ImGui::SetCursorPos(CurPos);

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
            }

            if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
            {
            }

            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
            }

            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Y"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Cut", "Ctrl+X"))
            {
            }

            if (ImGui::MenuItem("Copy", "Ctrl+C"))
            {
            }

            if (ImGui::MenuItem("Paste", "Ctrl+V"))
            {
            }

            //

            bool Serparated = false;
            for (auto& [ViewId, View] : m_Views)
            {
                if (View->OnMenuEdit())
                {
                    Serparated = true;
                }
            }

            if (!Serparated)
            {
                ImGui::Separator();
            }

            if (ImGui::MenuItem("Delete", "Del"))
            {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Views"))
        {
            for (auto& [ViewId, View] : m_Views)
            {
                bool IsOpen = IsViewOpen(ViewId);
                if (ImGui::MenuItem(View->GetWidgetId().c_str(), nullptr, IsOpen))
                {
                    if (IsOpen)
                    {
                        CloseView(ViewId);
                    }
                    else
                    {
                        OpenView(ViewId);
                    }
                }
            }

            ImGui::EndMenu();
        }

        for (auto& View : m_Views | std::views::values)
        {
            View->OnMenuBar();
        }

        m_IsTitlebarHovered &= !ImGui::IsAnyItemHovered();

        ImGui::PopStyleVar();
        ImGui::EndMainMenuBar();
    }

    void EditorEngine::EndEditorSpace()
    {
        ImGui::End();
    }
} // namespace Neon::Editor