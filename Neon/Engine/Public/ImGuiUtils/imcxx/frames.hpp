#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// start a new Dear ImGui frame, you can submit any command from this point until Render()/EndFrame().
	/// </summary>
	class [[nodiscard]] frame : public scope_wrap<frame, scope_traits::always_dtor, false>
	{
		friend class scope_wrap<frame, scope_traits::always_dtor, false>;
	public:
		template<typename _PlatNewFrameFnTy, typename = std::enable_if_t<std::is_invocable_v<_PlatNewFrameFnTy>>>
		frame(_PlatNewFrameFnTy plat_newframe)
		{
			plat_newframe();
			ImGui::NewFrame();
		}

	private:
		void destruct()
		{
			ImGui::EndFrame();
		}
	};

	/// <summary>
	/// ends the Dear ImGui frame, finalize the draw data.
	/// </summary>
	template<typename _PlatRenderFnTy, typename = std::enable_if_t<std::is_invocable_v<_PlatRenderFnTy, ImDrawData*>>>
	void render(_PlatRenderFnTy plat_render)
	{
		ImGui::Render();
		plat_render(ImGui::GetDrawData());
	}
}
