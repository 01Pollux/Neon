#pragma once

#include "scopes.hpp"

namespace imcxx
{
	class menuitem_entry;

	/// <summary>
	/// Widgets: Menus
	/// - Use BeginMenuBar() on a window ImGuiWindowFlags_MenuBar to append to its menu bar.
	/// - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
	/// - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same identifier to append more items to it.
	/// - Not that MenuItem() keyboardshortcuts are displayed as a convenience but _not processed_ by Dear ImGui at the moment.
	/// </summary>
	class [[nodiscard]] menubar : public scope_wrap<menubar>
	{
		friend class scope_wrap<menubar>;
	public:
		class item;
		class main_bar {};

		/// <summary>
		/// append to menu-bar of current window (requires ImGuiWindowFlags_MenuBar flag set on parent window).
		/// </summary>
		menubar() :
			scope_wrap(ImGui::BeginMenuBar()), m_IsMainBar(false)
		{}

		/// <summary>
		/// create and append to a full screen menu-bar.
		/// </summary>
		menubar(main_bar) :
			scope_wrap(ImGui::BeginMainMenuBar()), m_IsMainBar(true)
		{}


		/// <summary>
		/// create a sub-menu entry.
		/// </summary>
		template<typename _StrTy>
		[[nodiscard]] item add_item(const _StrTy& label, bool enabled = true)
		{
			return add_item_impl(impl::get_string(label), enabled);
		}
		
		/// <summary>
		/// return true when activated.
		/// </summary>
		template<typename _Str0Ty, typename _Str1Ty = const char*>
		menuitem_entry add_entry(const _Str0Ty& label, const _Str1Ty& shortcut = nullptr, bool selected = false, bool enabled = true)
		{
			return add_entry_impl(impl::get_string(label), impl::get_string(shortcut), selected, enabled);
		}

		/// <summary>
		/// return true when activated + toggle (*p_selected) if p_selected != nullptr
		/// </summary>
		template<typename _Str0Ty, typename _Str1Ty = const char*>
		menuitem_entry add_entry(const _Str0Ty& label, const _Str1Ty& shortcut, bool* p_selected, bool enabled = true)
		{
			return add_entry_impl(impl::get_string(label), impl::get_string(shortcut), p_selected, enabled);
		}

	private:
		bool m_IsMainBar : 1;
		void destruct()
		{
			if (!m_IsMainBar)
				ImGui::EndMenuBar();
			else
				ImGui::EndMainMenuBar();
		}

		item add_item_impl(const char* label, bool enabled = true);

		menuitem_entry add_entry_impl(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);

		menuitem_entry add_entry_impl(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);
	};


	using menubar_item = menubar::item;
	class [[nodiscard]] menubar::item : public scope_wrap<item>
	{
		friend class scope_wrap<item>;
	public:

		item(const char* label, bool enabled = true) :
			scope_wrap(ImGui::BeginMenu(label, enabled))
		{}

		item(std::string_view label, bool enabled = true) :
			item(label.data(), enabled)
		{}

		/// <summary>
		/// return true when activated.
		/// </summary>
		template<typename _Str0Ty, typename _Str1Ty = const char*>
		menuitem_entry add_entry(const _Str0Ty& label, const _Str1Ty& shortcut = nullptr, bool selected = false, bool enabled = true)
		{
			return add_entry_impl(impl::get_string(label), impl::get_string(shortcut), selected, enabled);
		}

		/// <summary>
		/// return true when activated + toggle (*p_selected) if p_selected != nullptr
		/// </summary>
		template<typename _Str0Ty, typename _Str1Ty = const char*>
		menuitem_entry add_entry(const _Str0Ty& label, const _Str1Ty& shortcut, bool* p_selected, bool enabled = true)
		{
			return add_entry_impl(impl::get_string(label), impl::get_string(shortcut), p_selected, enabled);
		}

	private:
		void destruct()
		{
			ImGui::EndMenu();
		}

		menuitem_entry add_entry_impl(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);

		menuitem_entry add_entry_impl(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);
	};


	class menuitem_entry : public scope_wrap<menuitem_entry, scope_traits::no_dtor>
	{
	public:
		menuitem_entry(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true) :
			scope_wrap(ImGui::MenuItem(label, shortcut,  selected, enabled))
		{}
		
		menuitem_entry(const char* label, const char* shortcut, bool* p_selected, bool enabled = true) :
			scope_wrap(ImGui::MenuItem(label, shortcut, p_selected, enabled))
		{}
		
		menuitem_entry(std::string_view label, std::string_view shortcut = "", bool selected = false, bool enabled = true) :
			menuitem_entry(label.data(), shortcut.empty() ? nullptr : shortcut.data(), selected, enabled)
		{}
		
		menuitem_entry(std::string_view label, std::string_view shortcut, bool* p_selected, bool enabled = true) :
			menuitem_entry(label.data(), shortcut.empty() ? nullptr : shortcut.data(), p_selected, enabled)
		{}
	};


	inline auto menubar::add_item_impl(const char* label, bool enabled) -> item
	{
		return item{ label, enabled };
	}


	inline auto menubar::add_entry_impl(const char* label, const char* shortcut, bool selected, bool enabled) -> menuitem_entry
	{
		return menuitem_entry{ label, shortcut, selected, enabled };
	}

	inline auto menubar::add_entry_impl(const char* label, const char* shortcut, bool* p_selected, bool enabled) -> menuitem_entry
	{
		return menuitem_entry{ label, shortcut, p_selected, enabled };
	}


	inline auto menubar_item::add_entry_impl(const char* label, const char* shortcut, bool selected, bool enabled) -> menuitem_entry
	{
		return menuitem_entry{ label, shortcut, selected, enabled };
	}

	inline auto menubar_item::add_entry_impl(const char* label, const char* shortcut, bool* p_selected, bool enabled) -> menuitem_entry
	{
		return menuitem_entry{ label, shortcut, p_selected, enabled };
	}
}
