#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Tab Bars, Tabs
	/// Note: Tabs are automatically created by the docking system. Use this to create tab bars/tabs yourself without docking being involved.
	/// </summary>
	class [[nodiscard]] tabbar : public scope_wrap<tabbar>
	{
		friend class scope_wrap<tabbar>;
	public:
		class item;

		template<typename _StrTy>
		tabbar(const _StrTy& name, ImGuiTabBarFlags flags = 0) :
			scope_wrap(ImGui::BeginTabBar(impl::get_string(name), flags))
		{}


		/// <summary>
		/// create a Tab. Returns true if the Tab is selected.
		/// </summary>
		template<typename _StrTy>
		[[nodiscard]] item add_item(const _StrTy& label, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0)
		{
			return add_item_impl(impl::get_string(label), p_open, flags);
		}

		/// <summary>
		/// create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar
		/// </summary>
		template<typename _StrTy>
		[[nodiscard]] bool add_button(const _StrTy& label, ImGuiTabItemFlags flags = 0)
		{
			return ImGui::TabItemButton(impl::get_string(label), flags);
		}

		/// <summary>
		/// notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.
		/// </summary>
		void notify_tab_closed(const char* tab_or_docked_window_label)
		{
			ImGui::SetTabItemClosed(tab_or_docked_window_label);
		}
		
	private:
		void destruct()
		{
			ImGui::EndTabBar();
		}

		item add_item_impl(const char* label, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0);
	};

	class [[nodiscard]] tabbar::item : public scope_wrap<item>
	{
		friend class scope_wrap<item>;
	public:
		template<typename _StrTy>
		item(const _StrTy& label, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0) :
			scope_wrap(ImGui::BeginTabItem(impl::get_string(label), p_open, flags))
		{}

	private:
		void destruct()
		{
			ImGui::EndTabItem();
		}
	};

	inline auto tabbar::add_item_impl(const char* label, bool* p_open, ImGuiTabItemFlags flags) -> item
	{
		return item{ label, p_open, flags };
	}
}
