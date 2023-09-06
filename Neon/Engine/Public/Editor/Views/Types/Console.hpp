#pragma once

#include <Editor/Views/View.hpp>

namespace Neon::Asset
{
    class IAsset;
}

namespace Neon::Editor::Views
{
    class Console : public IEditorView
    {
    public:
        Console();

        void OnUpdate() override;

        void OnRender() override;

    private:
        Ptr<Asset::IAsset> m_Asset;
    };
} // namespace Neon::Editor::Views