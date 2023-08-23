#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Disabling [BETA API]
	/// - Disable all user interactions and dim items visuals (applying style.DisabledAlpha over current colors)
	/// - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
	/// - BeginDisabled(false) essentially does nothing useful but is provided to facilitate use of boolean expressions. If you can avoid calling BeginDisabled(False)/EndDisabled() best to avoid it.
	/// </summary>
    class [[nodiscard]] disabled : public scope_wrap<disabled, imcxx::scope_traits::always_dtor, false>
	{
		friend class scope_wrap<disabled, imcxx::scope_traits::always_dtor, false>;
	public:
		disabled(bool disabled = true)
		{
			ImGui::BeginDisabled(disabled);
		}

	private:
		void destruct()
		{
			ImGui::EndDisabled();
		}
	};
}
