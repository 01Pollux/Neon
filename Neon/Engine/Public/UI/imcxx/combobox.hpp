#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Widgets: Combo Box
	/// - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.
	/// - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for convenience purpose. This is analogous to how ListBox are created.
	/// </summary>
	class [[nodiscard]] combo_box : public scope_wrap<combo_box>
	{
		friend class scope_wrap<combo_box>;
	public:
		combo_box(const char* label, const char* preview_value, ImGuiComboFlags flags = 0) :
			scope_wrap(ImGui::BeginCombo(label, preview_value, flags))
		{}

	private:
		void destruct()
		{
			ImGui::EndCombo();
		}
	};
}
