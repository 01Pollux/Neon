#include <EnginePCH.hpp>
#include <Editor/Views/Types/MaterialEditor.hpp>

#include <UI/MaterialEditor.hpp>
#include <UI/NodeBuilder.hpp>

//

namespace Neon::Editor::Views
{
    struct Pin
    {
        UI::NodeEditor::PinId Id;
        StringU8              Name;

        Pin(
            UI::NodeEditor& Editor,
            StringU8        Name) :
            Id(Editor.NewPinId()),
            Name(std::move(Name))
        {
        }
    };

    struct Node
    {
        UI::NodeEditor::NodeId Id;
        StringU8               Name;
        std::vector<Pin>       Inputs, Outputs;

        ImColor Color = ImColor(1.f, 1.f, 1.f, 1.f);

        Node(
            UI::NodeEditor& Editor,
            StringU8        Name) :
            Id(Editor.NewNodeId()),
            Name(std::move(Name))
        {
        }

        void AddInput(
            UI::NodeEditor& Editor,
            StringU8        Name)
        {
            Inputs.emplace_back(Editor, std::move(Name));
        }

        void AddOutput(
            UI::NodeEditor& Editor,
            StringU8        Name)
        {
            Outputs.emplace_back(Editor, std::move(Name));
        }
    };

    static std::vector<Node> s_Nodes;

    //

    MaterialEditor::MaterialEditor(
        const StringU8& TabName) :
        IEditorView(StringUtils::Format("Material Editor: {}", TabName))
    {
        m_NodeEditor = UI::NodeEditor::EditorConfig{};

        //

        auto& InputTexture = s_Nodes.emplace_back(m_NodeEditor, "Input: Texture");
        InputTexture.AddOutput(m_NodeEditor, "[0]");

        auto& InputUV = s_Nodes.emplace_back(m_NodeEditor, "Input: UV");
        InputUV.AddInput(m_NodeEditor, "Out");

        auto& SampleTexture = s_Nodes.emplace_back(m_NodeEditor, "Sample Texture");
        SampleTexture.AddInput(m_NodeEditor, "Texture");
        SampleTexture.AddInput(m_NodeEditor, "Sampler");
        SampleTexture.AddInput(m_NodeEditor, "UV");
        SampleTexture.AddOutput(m_NodeEditor, "Out");

        auto& OutputPixel = s_Nodes.emplace_back(m_NodeEditor, "Output: Pixel");
        OutputPixel.AddInput(m_NodeEditor, "Base Color (3)");
        OutputPixel.AddInput(m_NodeEditor, "Alpha (1)");
        OutputPixel.AddInput(m_NodeEditor, "Normal (3)");
        OutputPixel.AddInput(m_NodeEditor, "Emissive (4)");
    }

    void MaterialEditor::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        m_NodeEditor.Begin("Material Editor");
        if (0)
        {
            UI::NodeBuilder Builder(&m_NodeEditor);

            static Pin* newLinkPin = nullptr;

            for (auto& Node : s_Nodes)
            {
                Builder.Begin(Node.Id);

                {
                    Builder.Header(Node.Color);
                    ImGui::TextUnformatted(Node.Name.c_str(), Node.Name.c_str() + Node.Name.size());
                    ImGui::Dummy(ImVec2(0, 28));
                    Builder.EndHeader();
                }

                for (auto& Input : Node.Inputs)
                {
                    float Alpha = ImGui::GetStyle().Alpha;
                    /*   if (newLinkPin && !CanCreateLink(newLinkPin, &Input) && &Input != newLinkPin)
                           alpha *= (48.0f / 255.0f);*/

                    {
                        Builder.Input(Input.Id);
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Alpha);
                        ImGui::TextUnformatted(Input.Name.c_str());
                        ImGui::PopStyleVar();
                        Builder.EndInput();
                    }
                }

                for (auto& Output : Node.Outputs)
                {
                    float Alpha = ImGui::GetStyle().Alpha;
                    /*   if (newLinkPin && !CanCreateLink(newLinkPin, &Input) && &Input != newLinkPin)
                           alpha *= (48.0f / 255.0f);*/

                    {
                        Builder.Output(Output.Id);
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Alpha);
                        ImGui::TextUnformatted(Output.Name.c_str());
                        ImGui::PopStyleVar();
                        Builder.EndOutput();
                    }
                }

                Builder.End();
            }
        }
        m_NodeEditor.End();
    }
} // namespace Neon::Editor::Views