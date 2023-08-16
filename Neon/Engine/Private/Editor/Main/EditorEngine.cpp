#include <EnginePCH.hpp>
#include <Editor/Main/EditorEngine.hpp>
#include <Window/Window.hpp>

#include <Asset/Packs/Directory.hpp>

#include <imgui/imgui.h>
#include <Log/Logger.hpp>

namespace Neon::Editor
{
    static bool  show = true;
    static float colro[3]{ 0.1f, 0.1f, 0.1f };
    static float f       = 0.0f;
    static int   counter = 0;

    static EditorViewId s_EditorEngineId = 1;

    void EditorEngine::Initialize(
        Config::EngineConfig Config)
    {
        Config.Resource.AssetPackages.emplace_back(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));
        GameEngine::Initialize(std::move(Config));
    }

    void EditorEngine::PreUpdate()
    {

        GameEngine::PreUpdate();
        for (auto& View : m_OpenViews)
        {
            View->OnUpdate();
        }
    }

    void EditorEngine::PostRender()
    {
        GameEngine::PostRender();
        for (auto& View : m_OpenViews)
        {
            View->OnRender();
        }

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show)
            ImGui::ShowDemoWindow(&show);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show);    // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", colro);     // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::End();
        }
        ImGui::Begin("Another Window", &show); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show = false;
        ImGui::End();
    }

    //

    EditorViewId EditorEngine::RegisterView(
        UPtr<IEditorView> View)
    {
        NEON_ASSERT(s_EditorEngineId, "Editor view id overflow.");

        uint64_t Id = s_EditorEngineId++;
        m_Views.emplace(Id, std::move(View));

        return Id;
    }

    void EditorEngine::UnregisterView(
        EditorViewId ViewId)
    {
        auto Iter = m_Views.find(ViewId);
        if (Iter == m_Views.end())
        {
            NEON_WARNING("Editor", "Trying to unregister a view that does not exist.");
            return;
        }
        m_OpenViews.erase(Iter->second.get());
        m_Views.erase(Iter);
    }

    void EditorEngine::OpenView(
        EditorViewId ViewId)
    {
        auto Iter = m_Views.find(ViewId);
        if (Iter == m_Views.end())
        {
            NEON_WARNING("Editor", "Trying to open a view that does not exist.");
            return;
        }

        auto [View, ShouldOpen] = m_OpenViews.emplace(Iter->second.get());
        if (ShouldOpen)
        {
            Iter->second->OnOpen();
        }
    }

    bool EditorEngine::IsViewOpen(
        EditorViewId ViewId)
    {
        auto Iter = m_Views.find(ViewId);
        return Iter != m_Views.end() ? m_OpenViews.contains(Iter->second.get()) : false;
    }

    void EditorEngine::CloseView(
        EditorViewId ViewId)
    {
        auto Iter = m_Views.find(ViewId);
        if (Iter == m_Views.end())
        {
            NEON_WARNING("Editor", "Trying to close a view that does not exist.");
            return;
        }

        auto View = Iter->second.get();
        if (m_OpenViews.erase(View))
        {
            View->OnClose();
        }
    }
} // namespace Neon::Editor
