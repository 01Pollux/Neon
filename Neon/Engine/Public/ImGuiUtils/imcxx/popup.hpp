#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Popups, Modals
	///  - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
	///  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
	///  - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
	///  - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
	///  - You can bypass the hovering restriction by using ImGuiHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
	///  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.
	///    This is sometimes leading to confusing mistakes. May rework this in the future.
	/// </summary>
	class [[nodiscard]] popup : public scope_wrap<popup>
	{
		friend class scope_wrap<popup>;
	public:
		struct modal {};
		struct context_item {};
		struct context_void {};
		struct context_window {};

		/// <summary>
		/// Popups: begin/end functions
		///  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards. ImGuiWindowFlags are forwarded to the window.
		///  - BeginPopupModal(): block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar.
		/// </summary>
		template<typename _StrTy = const char*>
		popup(const _StrTy& str_id, ImGuiWindowFlags flags = 0) :
			scope_wrap(ImGui::BeginPopup(impl::get_string(str_id), flags))
		{}

		/// <summary>
		/// Popups: begin/end functions
		///  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards. ImGuiWindowFlags are forwarded to the window.
		///  - BeginPopupModal(): block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar.
		/// </summary>
		template<typename _StrTy = const char*>
		popup(modal, const _StrTy& str_id, bool* p_open = nullptr, ImGuiWindowFlags flags = 0) :
			scope_wrap(ImGui::BeginPopupModal(impl::get_string(str_id), p_open, flags))
		{}

		/// <summary>
		/// Popups: open+begin combined functions helpers
		///  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
		///  - They are convenient to easily create context menus, hence the name.
		///  - IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add ImGuiWindowFlags to the BeginPopupContextXXX functions in the future.
		///  - IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, 
		///  so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.
		/// </summary>
		template<typename _StrTy = const char*>
		popup(context_item, const _StrTy& str_id = nullptr, ImGuiWindowFlags flags = 1) :
			scope_wrap(ImGui::BeginPopupContextItem(impl::get_string(str_id), flags))
		{}

		/// <summary>
		/// Popups: open+begin combined functions helpers
		///  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
		///  - They are convenient to easily create context menus, hence the name.
		///  - IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add ImGuiWindowFlags to the BeginPopupContextXXX functions in the future.
		///  - IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, 
		///  so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.
		/// </summary>
		template<typename _StrTy = const char*>
		popup(context_void, const _StrTy& str_id = nullptr, ImGuiWindowFlags flags = 1) :
			scope_wrap(ImGui::BeginPopupContextVoid(impl::get_string(str_id), flags))
		{}

		/// <summary>
		/// Popups: open+begin combined functions helpers
		///  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
		///  - They are convenient to easily create context menus, hence the name.
		///  - IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add ImGuiWindowFlags to the BeginPopupContextXXX functions in the future.
		///  - IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, 
		///  so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.
		/// </summary>
		template<typename _StrTy = const char*>
		popup(context_window, const _StrTy& str_id = nullptr, ImGuiWindowFlags flags = 1) :
			scope_wrap(ImGui::BeginPopupContextWindow(impl::get_string(str_id), flags))
		{}

		/// <summary>
		/// Calls 'ImGui::CloseCurrentPopup()'
		/// </summary>
		void close()
		{
			ImGui::CloseCurrentPopup();
		}

	private:
		void destruct()
		{
			ImGui::EndPopup();
		}
	};
}
