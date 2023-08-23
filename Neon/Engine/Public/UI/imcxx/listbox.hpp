#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Widgets: List Boxes
	/// - This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.
	/// - The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() or any items.
	/// - The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept available for convenience purpose. This is analoguous to how Combos are created.
	/// - Choose frame width:   size.x > 0.0f: custom  /  size.x < 0.0f or -FLT_MIN: right-align   /  size.x = 0.0f (default): use current ItemWidth
	/// - Choose frame height:  size.y > 0.0f: custom  /  size.y < 0.0f or -FLT_MIN: bottom-align  /  size.y = 0.0f (default): arbitrary default height which can fit ~7 items
	/// </summary>
	class [[nodiscard]] list_box : public scope_wrap<list_box>
	{
		friend class scope_wrap<list_box>;
	public:
		list_box(const char* label, ImVec2 size = {}) :
			scope_wrap(ImGui::BeginListBox(label, size))
		{}
		
	private:
		void destruct()
		{
			ImGui::EndListBox();
		}
	};
}
