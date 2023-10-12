#include <EnginePCH.hpp>
#include <Editor/Views/Types/MaterialEditor.hpp>

//

namespace Neon::Editor::Views
{
    MaterialEditor::MaterialEditor(
        const StringU8& TabName) :
        IEditorView(StringUtils::Format("Material Editor: {}", TabName))
    {
        auto InputTexture = m_NodeGraph.AddNode<UI::Graph::Node>(
            "Input: Texture",
            UI::Graph::Node::Type::Blueprint);

        m_NodeGraph.AddPin(
            InputTexture,
            false,
            { "[0]", UI::Graph::Pin::Type::Object });

        auto InputUV = m_NodeGraph.AddNode<UI::Graph::Node>(
            "Input: UV",
            UI::Graph::Node::Type::Blueprint);

        m_NodeGraph.AddPin(
            InputUV,
            false,
            { "Out", UI::Graph::Pin::Type::Float });

        auto SampleTexture = m_NodeGraph.AddNode<UI::Graph::Node>(
            "Sample Texture",
            UI::Graph::Node::Type::Blueprint);

        m_NodeGraph.AddPin(
            SampleTexture,
            true,
            { "Texture", UI::Graph::Pin::Type::Object });
        m_NodeGraph.AddPin(
            SampleTexture,
            true,
            { "Sampler", UI::Graph::Pin::Type::Object });
        m_NodeGraph.AddPin(
            SampleTexture,
            true,
            { "UV", UI::Graph::Pin::Type::Float });
        m_NodeGraph.AddPin(
            SampleTexture,
            false,
            { "Out", UI::Graph::Pin::Type::Float });

        auto OutputPixel = m_NodeGraph.AddNode<UI::Graph::Node>(
            "Output: Pixel",
            UI::Graph::Node::Type::Blueprint);

        m_NodeGraph.AddPin(
            OutputPixel,
            true,
            { "Base Color (3)", UI::Graph::Pin::Type::Float });
        m_NodeGraph.AddPin(
            OutputPixel,
            true,
            { "Alpha (1)", UI::Graph::Pin::Type::Float });
        m_NodeGraph.AddPin(
            OutputPixel,
            true,
            { "Normal (3)", UI::Graph::Pin::Type::Float });
        m_NodeGraph.AddPin(
            OutputPixel,
            true,
            { "Emissive (4)", UI::Graph::Pin::Type::Float });
    }

    void MaterialEditor::OnRender()
    {
        imcxx::window Window(GetWidgetId(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        if (!Window)
        {
            return;
        }

        if (m_NodeGraph->Begin("Material Editor"))
        {
            m_NodeGraph.Render();
        }
        m_NodeGraph->End();
    }
} // namespace Neon::Editor::Views