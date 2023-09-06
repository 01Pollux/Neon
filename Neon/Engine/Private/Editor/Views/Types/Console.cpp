#include <EnginePCH.hpp>
#include <Editor/Views/Types/Console.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Asset/Types/Model.hpp>
#include <Asset/Handlers/Model.hpp>
#include <fstream>

namespace Neon::Editor::Views
{
    Console::Console() :
        IEditorView(StandardViews::s_ConsoleViewWidgetId)
    {
    }

    void Console::OnUpdate()
    {
    }

    void Console::OnRender()
    {
        imcxx::window Window{ GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse };
        if (!Window)
        {
            return;
        }

        if (ImGui::Button("Test import"))
        {
            Asset::ModelAsset::Handler Handler;

            Asset::DependencyReader Reader;

            std::ifstream FbxFile(R"(D:\Ph\Room #1.fbx)", std::ios::binary);

            auto Asset = std::dynamic_pointer_cast<Asset::ModelAsset>(
                Handler.Load(
                    FbxFile,
                    Reader,
                    Asset::Handle::Random(),
                    R"(D:\Ph\Room #1.fbx)",
                    {}));

            auto m = Asset->GetModel();

            auto p = Asset->GetGuid();
            // Project::Get()->ImportAsset();
        }
    }
} // namespace Neon::Editor::Views