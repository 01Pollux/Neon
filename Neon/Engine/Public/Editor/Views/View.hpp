#pragma once

#include <Core/String.hpp>
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

namespace Neon::Editor
{
    namespace StandardViews
    {
        static constexpr const char* s_HierarchyViewWidgetId  = "Hiearchy View";
        static constexpr const char* s_SceneViewWidgetId      = "Scene View";
        static constexpr const char* s_GameViewWidgetId       = "Game View";
        static constexpr const char* s_InspectorViewWidgetId  = "Inspector View";
        static constexpr const char* s_ContentBrowserWidgetId = "Content Browser";
        static constexpr const char* s_ConsoleViewWidgetId    = "Console View";
    } // namespace StandardViews

    class IEditorView
    {
    public:
        IEditorView(
            StringU8 WidgetId) :
            m_WidgetId(std::move(WidgetId))
        {
        }

        virtual ~IEditorView() = default;

        /// <summary>
        /// Called when the view is updated.
        /// </summary>
        virtual void OnUpdate()
        {
        }

        /// <summary>
        /// Called when the view is rendered in the edit bar.
        /// Returning true will insert a seperator after the view.
        /// </summary>
        virtual bool OnMenuEdit()
        {
            return false;
        }

        /// <summary>
        /// Called when the view is rendered.
        /// </summary>
        virtual void OnMenuBar()
        {
        }

        /// <summary>
        /// Called when the view is rendered.
        /// </summary>
        virtual void OnRender()
        {
        }

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

    public:
        /// <summary>
        /// Get widget name's id.
        /// </summary>
        const StringU8& GetWidgetId() const
        {
            return m_WidgetId;
        }

        /// <summary>
        /// Set widget name's id.
        /// </summary>
        void SetWidgetId(
            StringU8 WidgetId)
        {
            m_WidgetId = std::move(WidgetId);
        }

    private:
        StringU8 m_WidgetId;
    };
} // namespace Neon::Editor