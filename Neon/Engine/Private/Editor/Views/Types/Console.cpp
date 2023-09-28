#include <EnginePCH.hpp>
#include <Editor/Views/Types/Console.hpp>
#include <Editor/Main/EditorEngine.hpp>

#include <Asset/Types/Model.hpp>
#include <Asset/Handlers/Model.hpp>
#include <fstream>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>
#include <Scene/Component/Renderable.hpp>

namespace Neon::Editor::Views
{
    void BuildSceneModel(
        const Scene::EntityHandle& SceneRoot,
        const Scene::EntityHandle& ParentEntity,
        const Ptr<Mdl::Model>&     Model,
        uint32_t                   NodeIndex = 0)
    {
        auto& Node = Model->GetNode(NodeIndex);

        flecs::entity Entity = Scene::EntityHandle::Create(SceneRoot, ParentEntity, Node.Name.c_str());

        switch (Node.Submeshes.size())
        {
        // No submeshes, do nothing
        case 0:
            break;
        // If we have only one submesh, create it on top of the entity
        case 1:
        {
            Entity.emplace<Scene::Component::Transform>(Node.Transform);
            Entity.emplace<Scene::Component::MeshInstance>(Mdl::Mesh{ Model, Node.Submeshes[0] });
            Entity.emplace<Scene::Component::Renderable>();

            break;
        }
        // If we have more than one submesh, create a child entity for each submesh
        default:
        {
            size_t NameIndex = 0;
            for (Mdl::Model::SubmeshIndex SubmeshIdx : Node.Submeshes)
            {
                auto Name = StringUtils::Format("{}__Sub{}", Node.Name, NameIndex++);

                flecs::entity Child = Scene::EntityHandle::Create(SceneRoot, Entity, Name.c_str());
                Child.emplace<Scene::Component::Transform>(Node.Transform);
                Child.emplace<Scene::Component::MeshInstance>(Mdl::Mesh{ Model, SubmeshIdx });
                Child.emplace<Scene::Component::Renderable>();
            }

            break;
        }
        }

        for (auto& Child : Node.Children)
        {
            BuildSceneModel(SceneRoot, Entity, Model, Child);
        }
    }

    void CreateSceneModel(
        const Ptr<Mdl::Model>& Model)
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

        if (ImGui::Button("Test Import"))
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
        }
    }
} // namespace Neon::Editor::Views