#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Widgets: Trees
	/// - TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.
	/// </summary>
	class [[nodiscard]] tree_node : public scope_wrap<tree_node>
	{
		friend class scope_wrap<tree_node>;
	public:
		struct va_args {};
		struct push {};

		template<typename _StrTy>
		tree_node(const _StrTy& label, ImGuiTreeNodeFlags flags = 0) :
			scope_wrap(ImGui::TreeNodeEx(std::is_same_v<void*, _StrTy> ? label : impl::get_string(label), flags)),
			m_ShouldPop(should_tree_pop(flags))
		{}


		template<typename _StrTy, typename _FmtTy>
		tree_node(va_args, const _StrTy& label, ImGuiTreeNodeFlags flags, const _FmtTy& fmt, va_list arg_list) :
			m_ShouldPop(should_tree_pop(flags))
		{
			if constexpr (std::is_same_v<_StrTy, void*>)
				m_Result._Value = ImGui::TreeNodeExV(label, flags, impl::get_string(fmt), arg_list);
			else
				m_Result._Value = ImGui::TreeNodeExV(impl::get_string(label), 0, impl::get_string(fmt), arg_list);
		}
		

		template<typename _StrTy, typename _FmtTy, typename ..._Args>
		tree_node(const _StrTy& label, ImGuiTreeNodeFlags flags, const _FmtTy& fmt, _Args&&... args) :
			m_ShouldPop(should_tree_pop(flags))
		{
			if constexpr (std::is_same_v<_StrTy, void*> || std::is_same_v<_StrTy, const void*>)
				m_Result._Value = ImGui::TreeNodeEx(label, flags, impl::get_string(fmt), std::forward<_Args>(args)...);
			else
				m_Result._Value = ImGui::TreeNodeEx(impl::get_string(label), flags, impl::get_string(fmt), std::forward<_Args>(args)...);
		}


		template<typename _StrTy>
		tree_node(push, const _StrTy& label) :
			m_ShouldPop(true)
		{
			if constexpr (std::is_same_v<void*, _StrTy>)
				ImGui::TreePush(label);
			else
				ImGui::TreePush(impl::get_string(label));
		}


		/// <summary>
		/// set next TreeNode/CollapsingHeader open state.
		/// </summary>
		void open_next(bool is_open, ImGuiCond cond = 0)
		{
			ImGui::SetNextItemOpen(is_open, cond);
		}

	private:
		static constexpr bool should_tree_pop(ImGuiTreeNodeFlags flags) noexcept
		{
			return !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen);
		}

		void destruct()
		{
			if (m_ShouldPop)
				ImGui::TreePop();
		}
		
		bool m_ShouldPop : 1;
	};

	class collapsing_header : public scope_wrap<collapsing_header, scope_traits::no_dtor>
	{
	public:
		template<typename _StrTy>
		collapsing_header(const _StrTy& label, ImGuiTreeNodeFlags flags = 0) :
			scope_wrap(ImGui::CollapsingHeader(impl::get_string(label), flags))
		{}

		void open_next(bool is_open, ImGuiCond cond = 0)
		{
			ImGui::SetNextItemOpen(is_open, cond);
		}
	};
}
