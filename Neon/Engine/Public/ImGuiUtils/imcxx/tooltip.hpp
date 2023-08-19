#pragma once

#include "scopes.hpp"
#include "imgui/imgui_internal.h"

namespace imcxx
{
	/// <summary>
	/// Tooltips
	/// - Tooltip are windows following the mouse. They do not take focus away.
	/// </summary>
	class [[nodiscard]] tooltip : public scope_wrap<tooltip, scope_traits::always_dtor, false>
	{
		friend class scope_wrap<tooltip, scope_traits::always_dtor, false>;
	public:
		struct ignore_previous {};

		tooltip()
		{
			ImGui::BeginTooltip();
		}

		tooltip(ignore_previous)
		{
			ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePreviousTooltip, ImGuiWindowFlags_None);
		}

	private:
		void destruct()
		{
			ImGui::EndTooltip();
		}
	};
}
