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

    void EditorEngine::EndEditorSpace()
    {
        ImGui::End();
    }
} // namespace Neon::Editor