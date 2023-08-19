#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// lock and unlock horizontal starting position + capture the whole group bounding box into one "item"
	/// (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
	/// </summary>
	class [[nodiscard]] group : public scope_wrap<group, scope_traits::always_dtor, false>
	{
		friend class scope_wrap<group, scope_traits::always_dtor, false>;
	public:
		group()
		{
			ImGui::BeginGroup();
		}

	private:
		void destruct()
		{
			ImGui::EndGroup();
		}
	};

	/// <summary>
	/// move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
	/// </summary>
	class [[nodiscard]] indent : public scope_wrap<indent, scope_traits::has_dtor>
	{
		friend class scope_wrap<indent, scope_traits::has_dtor>;
	public:
		indent(float indent_w = 0.f, bool enable = true) : 
			scope_wrap(enable), m_Indent(indent_w)
		{
			if (enable)
				ImGui::Indent(indent_w);
		}

	private:
		void destruct()
		{
			ImGui::Unindent(m_Indent);
		}
		float m_Indent;
	};

	/// <summary>
	/// move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
	/// </summary>
	class [[nodiscard]] unindent : public scope_wrap<indent, scope_traits::has_dtor>
	{
		friend class scope_wrap<indent, scope_traits::has_dtor>;
	public:
		unindent(float indent_w = 0.f, bool enable = true) :
			scope_wrap(enable), m_Indent(indent_w)
		{
			if (enable)
				ImGui::Unindent(indent_w);
		}

	private:
		void destruct()
		{
			ImGui::Indent(m_Indent);
		}
		float m_Indent;
	};
}
