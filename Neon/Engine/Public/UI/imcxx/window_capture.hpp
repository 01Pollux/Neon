#pragma once

#include "window.hpp"

namespace imcxx
{
	using window_capture = window::capture;

	class [[nodiscard]] window::capture
	{
	public:
		struct read_or_write {};
		struct imid {};
		struct popup {};
		struct nav {};

		capture(ImGuiWindow* window) :
			m_Window(window)
		{}

		capture(read_or_write, bool read_only = true) :
			capture(!read_only ? ImGui::GetCurrentWindow() : ImGui::GetCurrentWindowRead())
		{}

		capture(imid, ImGuiID id) :
			capture(ImGui::FindWindowByID(id))
		{}

		capture(imid, const char* name) :
			capture(ImGui::FindWindowByName(name))
		{}

		capture(popup, bool visible) :
			capture(visible ? ImGui::GetTopMostAndVisiblePopupModal() : ImGui::GetTopMostPopupModal())
		{}

		capture(nav) :
			capture(ImGui::GetCurrentContext()->NavWindow)
		{}

	public:
		void set_focus_id(ImGuiID id)
		{
			ImGui::SetActiveID(id, get());
		}

		void set_active_id(ImGuiID id)
		{
			ImGui::SetFocusID(id, get());
		}

		void focus()
		{
			ImGui::FocusWindow(get());
		}

		void focus_above(ImGuiWindow* ignore_window = nullptr)
		{
			ImGui::FocusTopMostWindowUnderOne(get(), ignore_window);
		}

		void focus_front()
		{
			ImGui::BringWindowToFocusFront(get());
		}

		void display_front()
		{
			ImGui::BringWindowToDisplayFront(get());
		}

		void display_back()
		{
			ImGui::BringWindowToDisplayBack(get());
		}

		void display_below(ImGuiWindow* ignore_window = nullptr)
		{
			ImGui::BringWindowToDisplayBehind(get(), ignore_window);
		}

		[[nodiscard]] ImGuiWindow* bottom_most_visible() const
		{
			return ImGui::FindBottomMostVisibleWindowWithinBeginStack(get());
		}

		[[nodiscard]] int index() const
		{
			return ImGui::FindWindowDisplayIndex(get());
		}

	public:
		[[nodiscard]] bool is_appearing() const noexcept
		{
			return get()->Appearing;
		}

		[[nodiscard]] bool is_collapsed() const noexcept
		{
			return get()->Collapsed;
		}

		/// <summary>
		/// is current window focused? or its root/child, depending on flags. see flags for options.
		/// </summary>
		[[nodiscard]] bool is_focused(ImGuiFocusedFlags flags = 0) const noexcept
		{
			ImGuiWindow* ref_window = ImGui::GetCurrentContext()->NavWindow;
			ImGuiWindow* cur_window = get();

			if (!ref_window)
				return false;
			if (flags & ImGuiFocusedFlags_AnyWindow)
				return true;

			IM_ASSERT(cur_window); // Not inside a Begin()/End()
			const bool popup_hierarchy = (flags & ImGuiFocusedFlags_NoPopupHierarchy) == 0;
			const bool dock_hierarchy = (flags & ImGuiFocusedFlags_DockHierarchy) != 0;
			if (flags & ImGuiHoveredFlags_RootWindow)
				cur_window = get_combined_root_window(cur_window, popup_hierarchy, dock_hierarchy);

			if (flags & ImGuiHoveredFlags_ChildWindows)
				return is_window_child_of(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
			else
				return (ref_window == cur_window);
		}

		/// <summary>
		/// is current window hovered (and typically: not blocked by a popup/modal)? see flags for options. 
		/// NB: If you are trying to check whether your mouse should be dispatched to imgui or to your app,
		/// you should use the 'io.WantCaptureMouse' boolean for that! Please read the FAQ!
		/// </summary>
		[[nodiscard]] bool is_hovered(ImGuiHoveredFlags flags = 0) const noexcept
		{
			IM_ASSERT((flags & (ImGuiHoveredFlags_AllowWhenOverlapped | ImGuiHoveredFlags_AllowWhenDisabled)) == 0);   // Flags not supported by this function
			ImGuiContext& g = *GImGui;
			ImGuiWindow* ref_window = g.HoveredWindow;
			ImGuiWindow* cur_window = g.CurrentWindow;
			if (ref_window == NULL)
				return false;

			if (!(flags & ImGuiHoveredFlags_AnyWindow))
			{
				IM_ASSERT(cur_window); // Not inside a Begin()/End()
				const bool popup_hierarchy = (flags & ImGuiHoveredFlags_NoPopupHierarchy) == 0;
				const bool dock_hierarchy = (flags & ImGuiHoveredFlags_DockHierarchy) != 0;
				if (flags & ImGuiHoveredFlags_RootWindow)
					cur_window = get_combined_root_window(cur_window, popup_hierarchy, dock_hierarchy);

				bool result;
				if (flags & ImGuiHoveredFlags_ChildWindows)
					result = is_window_child_of(ref_window, cur_window, popup_hierarchy, dock_hierarchy);
				else
					result = (ref_window == cur_window);
				if (!result)
					return false;
			}

			if (!is_window_content_hoverable(ref_window, flags))
				return false;
			if (!(flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
				if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap && g.ActiveId != ref_window->MoveId)
					return false;
			return true;
		}

		/// <summary>
		/// get viewport currently associated to the current window.
		/// </summary>
		[[nodiscard]] ImGuiViewport* viewport() const noexcept
		{
			return get()->Viewport;
		}

	public:
		void update_root(ImGuiWindowFlags flags, ImGuiWindow* parent_window) const
		{
			ImGui::UpdateWindowParentAndRootLinks(get(), flags, parent_window);
		}

		void calc_next_autofit_size() const
		{
			ImGui::CalcWindowNextAutoFitSize(get());
		}

		[[nodiscard]] bool is_parent_of(ImGuiWindow* potential_child, bool popup_hierarchy, bool dock_hierarchy) const
		{
			return ImGui::IsWindowChildOf(potential_child, get(), popup_hierarchy, dock_hierarchy);
		}

		[[nodiscard]] bool is_child_of(ImGuiWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy) const
		{
			return ImGui::IsWindowChildOf(get(), potential_parent, popup_hierarchy, dock_hierarchy);
		}

		[[nodiscard]] bool within_beginstack(ImGuiWindow* potential_parent) const
		{
			return ImGui::IsWindowWithinBeginStackOf(get(), potential_parent);
		}

		[[nodiscard]] bool is_above(ImGuiWindow* potential_below) const
		{
			return ImGui::IsWindowAbove(get(), potential_below);
		}

		[[nodiscard]] bool is_below(ImGuiWindow* potential_above) const
		{
			return ImGui::IsWindowAbove(potential_above, get());
		}

		[[nodiscard]] bool is_navfocusable() const
		{
			return ImGui::IsWindowNavFocusable(get());
		}

		void set_pos(const ImVec2& pos, ImGuiCond cond = 0)
		{
			ImGui::SetWindowPos(get(), pos, cond);
		}

		void set_size(const ImVec2& pos, ImGuiCond cond = 0)
		{
			ImGui::SetWindowSize(get(), pos, cond);
		}

		void collapse(bool collapsed, ImGuiCond cond = 0)
		{
			ImGui::SetWindowCollapsed(get(), collapsed, cond);
		}

		void set_fontscale(float scale)
		{
			get()->FontWindowScale = scale;
			ImGui::GetCurrentContext()->FontSize = ImGui::GetCurrentContext()->DrawListSharedData.FontSize = get()->CalcFontSize();
		}

		void hittest_hole(const ImVec2& pos, const ImVec2& size)
		{
			ImGui::SetWindowHitTestHole(get(), pos, size);
		}

		[[nodiscard]] ImRect abs_to_rel(const ImRect& rect) const
		{
			return ImGui::WindowRectAbsToRel(get(), rect);
		}

		[[nodiscard]] ImRect rel_to_abs(const ImRect& rect) const
		{
			return ImGui::WindowRectRelToAbs(get(), rect);
		}

	public:
		void scroll_x(float x)
		{
			ImGui::SetScrollX(get(), x);
		}

		void scroll_y(float y)
		{
			ImGui::SetScrollY(get(), y);
		}

		void scroll_x_rel(float local_x, float center_x_ratio)
		{
			ImGui::SetScrollFromPosX(get(), local_x, center_x_ratio);
		}

		void scroll_y_rel(float local_y, float center_y_ratio)
		{
			ImGui::SetScrollFromPosY(get(), local_y, center_y_ratio);
		}

		void scroll_x_here(float center_x_ratio)
		{
			ImGuiContext* ctx = ImGui::GetCurrentContext();
			float spacing_x = (std::max) (get()->WindowPadding.x, ctx->Style.ItemSpacing.x);
			float target_pos_x = ImLerp(ctx->LastItemData.Rect.Min.x - spacing_x, ctx->LastItemData.Rect.Max.x + spacing_x, center_x_ratio);
			scroll_x_rel(target_pos_x - get()->Pos.x, center_x_ratio); // Convert from absolute to local pos

			// Tweak: snap on edges when aiming at an item very close to the edge
			get()->ScrollTargetEdgeSnapDist.x = (std::max) (0.f, get()->WindowPadding.x - spacing_x);
		}

		void scroll_y_here(float center_y_ratio)
		{
			float spacing_y = (std::max) (get()->WindowPadding.y, ImGui::GetCurrentContext()->Style.ItemSpacing.x);
			float target_pos_y = ImLerp(get()->DC.CursorPosPrevLine.y - spacing_y, get()->DC.CursorPosPrevLine.y + get()->DC.PrevLineSize.y + spacing_y, center_y_ratio);
			scroll_x_rel(target_pos_y - get()->Pos.y, center_y_ratio); // Convert from absolute to local pos

			// Tweak: snap on edges when aiming at an item very close to the edge
			get()->ScrollTargetEdgeSnapDist.y = (std::max) (0.f, get()->WindowPadding.y - spacing_y);
		}

		ImVec2 scroll(const ImRect& rect, ImGuiScrollFlags flags = 0)
		{
			return ImGui::ScrollToRectEx(get(), rect, flags);
		}

		void scroll_to_view(const ImRect& rect)
		{
			scroll(rect, ImGuiScrollFlags_KeepVisibleEdgeY);
		}

	public:
		// Content region
		// - Retrieve available space from a given point. GetContentRegionAvail() is frequently useful.
		// - Those functions are bound to be redesigned (they are confusing, incomplete and the Min/Max return values are in local window coordinates which increases confusion)

		ImVec2 abs_region_max() const noexcept
		{
			ImVec2 mx = get()->ContentRegionRect.Max;
			if (get()->DC.CurrentColumns || ImGui::GetCurrentContext()->CurrentTable)
				mx.x = get()->WorkRect.Max.x;
			return mx;
		}

		/// <summary>
		/// == GetContentRegionMax() - GetCursorPos()
		/// </summary>
		ImVec2 region_avail() const noexcept
		{
			ImVec2 pos = abs_region_max();
			pos.x -= get()->DC.CursorPos.x;
			pos.y -= get()->DC.CursorPos.y;
			return pos;
		}

		/// <summary>
		/// current content boundaries (typically window boundaries including scrolling, or current column boundaries), in windows coordinates
		/// </summary>
		ImVec2 region_max() const noexcept
		{
			ImVec2 mx{ get()->ContentRegionRect.Max.x - get()->Pos.x, get()->ContentRegionRect.Max.y - get()->Pos.y };
			if (get()->DC.CurrentColumns || ImGui::GetCurrentContext()->CurrentTable)
				mx.x = get()->WorkRect.Max.x - get()->Pos.x;
			return mx;
		}

		/// <summary>
		/// content boundaries min for the full window (roughly (0,0)-Scroll), in window coordinates
		/// </summary>
		ImVec2 content_region_min() const noexcept
		{
			return { get()->ContentRegionRect.Min.x - get()->Pos.x, get()->ContentRegionRect.Min.y - get()->Pos.y };
		}

		/// <summary>
		/// content boundaries min for the full window (roughly (0,0)-Scroll), in window coordinates
		/// </summary>
		ImVec2 content_region_max() const noexcept
		{
			return { get()->ContentRegionRect.Max.x - get()->Pos.x, get()->ContentRegionRect.Max.y - get()->Pos.y };
		}

	public:
		[[nodiscard]] ImDrawList* fg_drawlist() const
		{
			return ImGui::GetForegroundDrawList(get()->Viewport);
		}

		[[nodiscard]] ImDrawList* bg_drawlist() const
		{
			return ImGui::GetBackgroundDrawList(get()->Viewport);
		}

	private:
		[[nodiscard]] void close_popups(bool restore_focus_to_window_under_popup = true)
		{
			ImGui::ClosePopupsOverWindow(get(), restore_focus_to_window_under_popup);
		}

		[[nodiscard]] ImVec2 best_popup_pos() const
		{
			return ImGui::FindBestWindowPosForPopup(get());
		}

		[[nodiscard]] ImRect get_popup_extent() const
		{
			return ImGui::GetPopupAllowedExtentRect(get());
		}

	public:
		[[nodiscard]] operator bool() const noexcept
		{
			return m_Window != nullptr;
		}

		[[nodiscard]] ImGuiWindow* get() const noexcept
		{
			return m_Window;
		}

		[[nodiscard]] ImGuiWindow* get() noexcept
		{
			return m_Window;
		}

		[[nodiscard]] const ImGuiWindow* operator->() const noexcept
		{
			return get();
		}

		[[nodiscard]] ImGuiWindow* operator->() noexcept
		{
			return get();
		}

	private:
		[[nodiscard]] static ImGuiWindow* get_combined_root_window(ImGuiWindow* wnd, bool popup_hierarchy, bool dock_hierarchy)
		{
			ImGuiWindow* last_window = nullptr;
			while (last_window != wnd)
			{
				last_window = wnd;
				wnd = wnd->RootWindow;
				if (popup_hierarchy)
					wnd = wnd->RootWindowPopupTree;
				if (dock_hierarchy)
					wnd = wnd->RootWindowDockTree;
			}
			return wnd;
		}

		[[nodiscard]] static bool is_window_child_of(ImGuiWindow* window, ImGuiWindow* potential_parent, bool popup_hierarchy, bool dock_hierarchy)
		{
			ImGuiWindow* window_root = get_combined_root_window(window, popup_hierarchy, dock_hierarchy);
			if (window_root == potential_parent)
				return true;
			while (window)
			{
				if (window == potential_parent)
					return true;
				if (window == window_root) // end of chain
					return false;
				window = window->ParentWindow;
			}
			return false;
		}

		[[nodiscard]] static bool is_window_content_hoverable(ImGuiWindow* window, ImGuiHoveredFlags flags)
		{
			// An active popup disable hovering on other windows (apart from its own children)
			// FIXME-OPT: This could be cached/stored within the window.
			ImGuiContext* ctx = ImGui::GetCurrentContext();
			if (ctx->NavWindow)
			{
				if (ImGuiWindow* focused_root_window = ctx->NavWindow->RootWindowDockTree)
				{
					if (focused_root_window->WasActive && focused_root_window != window->RootWindowDockTree)
					{
						// For the purpose of those flags we differentiate "standard popup" from "modal popup"
						// NB: The order of those two tests is important because Modal windows are also Popups.
						if (focused_root_window->Flags & ImGuiWindowFlags_Modal)
							return false;
						if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiHoveredFlags_AllowWhenBlockedByPopup))
							return false;
					}
				}
			}

			// Filter by viewport
			if (window->Viewport != ctx->MouseViewport)
			{
				if (!ctx->MovingWindow || window->RootWindowDockTree != ctx->MovingWindow->RootWindowDockTree)
					return false;
			}

			return true;
		}

	private:
		ImGuiWindow* m_Window;
	};
}