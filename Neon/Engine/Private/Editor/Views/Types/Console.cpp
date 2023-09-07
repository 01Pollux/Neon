#include <EnginePCH.hpp>
#include <Editor/Views/Types/Console.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Asset/Types/Model.hpp>
#include <Asset/Handlers/Model.hpp>
#include <fstream>

#include <Scene/Component/Transform.hpp>

namespace Neon::Editor::Views
{
    void BuildSceneModel(
        const Scene::EntityHandle&  SceneRoot,
        const Scene::EntityHandle&  ParentEntity,
        const Ptr<Renderer::Model>& Model,
        uint32_t                    NodeIndex = 0)
    {
        auto& Node = Model->GetNode(NodeIndex);

        flecs::entity Entity = Scene::EntityHandle::Create(SceneRoot, ParentEntity, Node.Name.c_str());

        Entity.emplace<Scene::Component::Transform>(Node.Transform);

        for (auto& Child : Node.Children)
        {
            BuildSceneModel(SceneRoot, Entity, Model, Child);
        }
    }

    void CreateSceneModel(
        const Ptr<Renderer::Model>& Model)
    {
        auto& CurScene = Project::Get()->GetActiveScene()->GetScene();

        BuildSceneModel(CurScene.GetRoot(), CurScene.GetRoot(), Model);
    }

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

            std::ifstream FbxFile(R"(D:\Ph\TestScene.fbx)", std::ios::binary);

            auto Asset = std::dynamic_pointer_cast<Asset::ModelAsset>(
                Handler.Load(
                    FbxFile,
                    Reader,
                    Asset::Handle::Random(),
                    R"(D:\Ph\TestScene.fbx)",
                    {}));

            m_Asset = Asset;

            CreateSceneModel(Asset->GetModel());

            auto p = Asset->GetGuid();
            // Project::Get()->ImportAsset();
        }
    }
} // namespace Neon::Editor::Views