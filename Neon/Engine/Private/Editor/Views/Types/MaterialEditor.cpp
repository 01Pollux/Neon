#include <EnginePCH.hpp>
#include <Editor/Views/Types/MaterialEditor.hpp>

#include <UI/MaterialEditor.hpp>
#include <UI/NodeBuilder.hpp>

//

namespace Neon::Editor::Views
{
    struct Link
    {
        UI::NodeEditor::LinkId ID;

        UI::NodeEditor::PinId Source;
        UI::NodeEditor::PinId Dest;

        ImColor Color;
        float   Thickness = 1.f;

        Link(
            UI::NodeEditor&       Editor,
            UI::NodeEditor::PinId Source,
            UI::NodeEditor::PinId Dest) :
            ID(Editor.NewLinkId()),
            Source(Source),
            Dest(Dest), Color(255, 255, 255)
        {
        }
    };

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
    static std::vector<Link> s_Links;

    std::pair<Pin*, bool> FindPin(
        UI::NodeEditor::PinId Pin)
    {
        if (!Pin)
        {
            return {};
        }

        for (auto& Node : s_Nodes)
        {
            for (auto& Input : Node.Inputs)
            {
                if (Input.Id == Pin)
                {
                    return { &Input, true };
                }
            }
            for (auto& Output : Node.Outputs)
            {
                if (Output.Id == Pin)
                {
                    return { &Output, false };
                }
            }
        }

        return {};
    }

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
        InputUV.AddOutput(m_NodeEditor, "Out");

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

        if (m_NodeEditor.Begin("Material Editor"))
        {
            UI::NodeBuilder Builder(&m_NodeEditor);

            static Pin* NewLinkPin     = nullptr;
            static Pin* NewNodeLinkPin = nullptr;
            static bool CreateNewNode  = false;

            for (auto& Node : s_Nodes)
            {
                Builder.Begin(Node.Id);

                {
                    Builder.Header(Node.Color);
                    ImGui::TextUnformatted(Node.Name.c_str(), Node.Name.c_str() + Node.Name.size());
                    ImGui::Dummy(ImVec2(0, 28));
                    Builder.EndHeader();
                }

                if (imcxx::window_child{ "LeftSide", {}, true, ImGuiWindowFlags_AlwaysAutoResize })
                {
                    for (auto& Input : Node.Inputs)
                    {
                        float Alpha = ImGui::GetStyle().Alpha;
                        /*   if (newLinkPin && !CanCreateLink(newLinkPin, &Input) && &Input != newLinkPin)
                               alpha *= (48.0f / 255.0f);*/

                        {
                            Builder.Input(Input.Id);
                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Alpha);
                            UI::Utils::DrawIcon({ 24, 24 }, UI::Utils::BasicIconType::Circle, false, ImColor(220, 48, 48), ImColor(32.f, 32.f, 32.f, Alpha));
                            ImGui::SameLine();
                            ImGui::TextUnformatted(Input.Name.c_str());
                            ImGui::PopStyleVar();
                            Builder.EndInput();
                        }
                    }
                }

                ImGui::SameLine();
                {
                    imcxx::group LockHorz;
                    if (imcxx::window_child{ "RightSide", {}, true, ImGuiWindowFlags_AlwaysAutoResize })
                    {
                        for (auto& Output : Node.Outputs)
                        {
                            float Alpha = ImGui::GetStyle().Alpha;
                            /*   if (newLinkPin && !CanCreateLink(newLinkPin, &Input) && &Input != newLinkPin)
                                   alpha *= (48.0f / 255.0f);*/

                            {
                                Builder.Output(Output.Id);
                                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Alpha);
                                ImGui::TextUnformatted(Output.Name.c_str());
                                ImGui::SameLine();
                                UI::Utils::DrawIcon({ 24, 24 }, UI::Utils::BasicIconType::Circle, false, ImColor(220, 48, 48), ImColor(32.f, 32.f, 32.f, Alpha));
                                ImGui::PopStyleVar();
                                Builder.EndOutput();
                            }
                        }
                    }
                }

                Builder.End();
            }

            for (auto& Link : s_Links)
            {
                m_NodeEditor.Link(Link.ID, Link.Source, Link.Dest, Link.Color, Link.Thickness);
            }

            if (!CreateNewNode)
            {
                if (m_NodeEditor.BeginCreate())
                {
                    auto showLabel = [](const char* label, ImColor color)
                    {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                        auto size = ImGui::CalcTextSize(label);

                        auto padding = ImGui::GetStyle().FramePadding;
                        auto spacing = ImGui::GetStyle().ItemSpacing;

                        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                        auto rectMin = ImGui::GetCursorScreenPos() - padding;
                        auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
                        ImGui::TextUnformatted(label);
                    };

                    UI::NodeEditor::PinId StartPinId = 0, EndPinId = 0;
                    if (m_NodeEditor.QueryNewLink(&StartPinId, &EndPinId))
                    {
                        auto [StartPin, StartIsInput] = FindPin(StartPinId);
                        auto [EndPin, EndIsInput]     = FindPin(EndPinId);

                        NewLinkPin = StartPin ? StartPin : EndPin;
                        if (StartIsInput)
                        {
                            std::swap(EndPin, StartPin);
                            std::swap(StartPinId, EndPinId);
                        }

                        if (StartPin && EndPin)
                        {
                            if (StartPin == EndPin)
                            {
                                m_NodeEditor.RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            else if (StartIsInput == EndIsInput)
                            {
                                showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                                m_NodeEditor.RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            else
                            {
                                showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                                if (m_NodeEditor.AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                                {
                                    s_Links.emplace_back(m_NodeEditor, StartPinId, EndPinId);
                                }
                            }
                        }
                    }

                    UI::NodeEditor::PinId PinId = 0;
                    if (m_NodeEditor.QueryNewNode(&PinId))
                    {
                        std::tie(NewLinkPin, std::ignore) = FindPin(PinId);
                        if (NewLinkPin)
                            showLabel("+ Create Node", ImColor(32, 45, 32, 180));

                        if (m_NodeEditor.AcceptNewItem())
                        {
                            std::tie(NewNodeLinkPin, std::ignore) = FindPin(PinId);

                            CreateNewNode = true;
                            NewLinkPin    = nullptr;

                            m_NodeEditor.Suspend();
                            ImGui::OpenPopup("Create New Node");
                            m_NodeEditor.Resume();
                        }
                    }
                }
                else
                {
                    NewLinkPin = nullptr;
                }

                m_NodeEditor.EndCreate();
            }
        }
        m_NodeEditor.End();
    }
} // namespace Neon::Editor::Views