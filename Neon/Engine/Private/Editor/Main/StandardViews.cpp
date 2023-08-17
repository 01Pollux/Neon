#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Editor/Views/Types/ContentBrowser.hpp>

namespace Neon::Editor
{
    void EditorEngine::AddStandardViews()
    {
        RegisterView<Views::ContentBrowser>("_ContentBrowser0", StandardViews::s_HierarchyViewWidgetId);
        RegisterView<Views::ContentBrowser>("_ContentBrowser1", StandardViews::s_SceneViewWidgetId);
        RegisterView<Views::ContentBrowser>("_ContentBrowser2", StandardViews::s_GameViewWidgetId);
        RegisterView<Views::ContentBrowser>("_ContentBrowser3", StandardViews::s_InspectorViewWidgetId);
        RegisterView<Views::ContentBrowser>("_ContentBrowser4", StandardViews::s_ContentBrowserWidgetId);
        RegisterView<Views::ContentBrowser>("_ContentBrowser5", StandardViews::s_ConsoleViewWidgetId);

        OpenView("_ContentBrowser0");
        OpenView("_ContentBrowser1");
        OpenView("_ContentBrowser2");
        OpenView("_ContentBrowser3");
        OpenView("_ContentBrowser4");
        OpenView("_ContentBrowser5");
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

        constexpr ImGuiDockNodeFlags MainEditorWindowDockSpaceFlags =
            ImGuiDockNodeFlags_None |
            ImGuiDockNodeFlags_PassthruCentralNode;

        const ImGuiViewport* Viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(Viewport->WorkPos);
        ImGui::SetNextWindowSize(Viewport->WorkSize);
        ImGui::SetNextWindowViewport(Viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

        bool EditorOpen = ImGui::Begin("Neon Editor", nullptr, EditorWindowFlags);
        ImGui::PopStyleVar(2);

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

            ImGuiID TopCenter = ImGui::DockBuilderSplitNode(Center, ImGuiDir_Up, .65f, nullptr, &Center);
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
             * E = Content browser
             * F = Console view
             */

            ImGui::DockBuilderDockWindow(StandardViews::s_HierarchyViewWidgetId, Left);
            ImGui::DockBuilderDockWindow(StandardViews::s_SceneViewWidgetId, TopCenter);
            ImGui::DockBuilderDockWindow(StandardViews::s_GameViewWidgetId, TopCenter);
            ImGui::DockBuilderDockWindow(StandardViews::s_InspectorViewWidgetId, Right);
            ImGui::DockBuilderDockWindow(StandardViews::s_ContentBrowserWidgetId, Bottom);
            ImGui::DockBuilderDockWindow(StandardViews::s_ConsoleViewWidgetId, Bottom);

            ImGui::DockBuilderFinish(DockerspaceId);
        }

        ImGui::DockSpace(DockerspaceId, {}, MainEditorWindowDockSpaceFlags);
        return EditorOpen;
    }

    void EditorEngine::RenderMenuBar()
    {
        {
            ImVec2 FramePadding = ImGui::GetStyle().FramePadding;
            FramePadding.x *= 1.5f;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, FramePadding);
        }

        bool DisplayMenuBar;
        {
            ImVec2 FramePadding = ImGui::GetStyle().FramePadding;
            FramePadding.y *= 2.8f;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, FramePadding);
            DisplayMenuBar = ImGui::BeginMainMenuBar();
            ImGui::PopStyleVar();
        }

        if (!DisplayMenuBar)
        {
            return;
        }

        // Check if we are holding down mouse inside the menu bar
        // This is used for the window movement logic
        m_IsTitlebarHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        constexpr ImU32 HoveredColor = IM_COL32(0, 0, 0, 80);

        // Write title at top left with a some paddings
        {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HoveredColor);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 5.f));
            ImGui::Text("Neon Editor");
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        if (ImGui::BeginMenu("File"))
        {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HoveredColor);

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

            ImGui::PopStyleColor();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HoveredColor);

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

            ImGui::PopStyleColor();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Views"))
        {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HoveredColor);

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

            ImGui::PopStyleColor();
            ImGui::EndMenu();
        }

        for (auto& View : m_Views | std::views::values)
        {
            View->OnMenuBar();
        }

        ImGui::PopStyleVar();
        ImGui::EndMainMenuBar();
    }

    void EditorEngine::EndEditorSpace()
    {
        ImGui::End();
    }
} // namespace Neon::Editor