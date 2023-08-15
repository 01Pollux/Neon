#pragma once

namespace Neon::Editor
{
    class IEditorView
    {
    public:
        virtual ~IEditorView() = default;

        /// <summary>
        /// Called when the view is updated.
        /// </summary>
        virtual void OnUpdate() = 0;

        /// <summary>
        /// Called when the view is rendered.
        /// </summary>
        virtual void OnRender() = 0;

        /// <summary>
        /// Called when the view is opened.
        /// </summary>
        virtual void OnOpen()
        {
        }

        /// <summary>
        /// Called when the view is closed.
        /// </summary>
        virtual void OnClose()
        {
        }
    };
} // namespace Neon::Editor