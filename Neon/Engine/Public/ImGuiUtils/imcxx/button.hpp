#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Widgets: Main
	/// - Most widgets return true when the value has been changed or when pressed/selected
	/// - You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.
	/// </summary>
	class button : public scope_wrap<button, scope_traits::no_dtor>
	{
	public:
		struct small {};
		struct invisible {};
		struct arrow {};
		struct image {};
		struct radio {};
		struct progress {};	
		struct bullet {};	
		struct color {};

		template<typename _StrTy>
		button(const _StrTy& label, const ImVec2& size = {}) :
			scope_wrap(ImGui::Button(impl::get_string(label), size))
		{}


		/// <summary>
		/// button with FramePadding=(0,0) to easily embed within text
		/// </summary>
		template<typename _StrTy>
		button(small, const _StrTy& label) :
			scope_wrap(ImGui::SmallButton(impl::get_string(label)))
		{}

		
		/// <summary>
		/// flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
		/// </summary>
		template<typename _StrTy>
		button(invisible, const _StrTy& label, const ImVec2& size = {}, ImGuiButtonFlags flags = 0) :
			scope_wrap(ImGui::InvisibleButton(impl::get_string(label), size, flags))
		{}


		/// <summary>
		/// square button with an arrow shape
		/// </summary>
		template<typename _StrTy>
		button(arrow, const _StrTy& label, ImGuiDir direction) :
			scope_wrap(ImGui::ArrowButton(impl::get_string(label), direction))
		{}

		/// <summary>
		/// <0 frame_padding uses default frame padding settings. 0 for no padding
		/// </summary>
		button(
			image, 
			ImTextureID user_texture_id,
			const ImVec2& size,
			const ImVec2& uv0 = {}, 
			const ImVec2& uv1 = { 1.f, 1.f }, 
			int frame_padding = -1, 
			const ImVec4& bg_col = {  },
			const ImVec4& tint_col = { 1.f, 1.f, 1.f, 1.f }
		) :
			scope_wrap(ImGui::ImageButton(user_texture_id, size, uv0, uv1, frame_padding, bg_col, tint_col))
		{}


		/// <summary>
		/// use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
		/// </summary>
		template<typename _StrTy>
		button(radio, const _StrTy& label, bool active) :
			scope_wrap(ImGui::RadioButton(impl::get_string(label), active))
		{}

		/// <summary>
		/// shortcut to handle the above pattern when value is an integer
		/// </summary>
		template<typename _StrTy>
		button(radio, const _StrTy& label, int* value, int pos) :
			scope_wrap(ImGui::RadioButton(impl::get_string(label), value, pos))
		{}

		/// <summary>
		/// draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses
		/// </summary>
		button(bullet)
		{
			ImGui::Bullet();
		}


		/// <summary>
		/// draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses
		/// </summary>
		button(progress, float fraction, ImVec2 size_arg = { -FLT_MIN, 0.f }, const char* overlay = nullptr)
		{
			ImGui::ProgressBar(fraction, size_arg, overlay);
		}

		/// <summary>
		/// shortcut to handle the above pattern when value is an integer
		/// </summary>
		button(progress, float fraction, ImVec2 size_arg = { -FLT_MIN, 0.f }, const std::string_view overlay = "") :
			button(progress{}, fraction, size_arg, overlay.empty() ? nullptr : overlay.data())
		{}


		/// <summary>
		/// display a color square/button, hover for details, return true when pressed
		/// </summary>
		template<typename _StrTy>
		button(color, const _StrTy& desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, ImVec2 size = { }) :
			scope_wrap(ImGui::ColorButton(impl::get_string(desc_id), col, flags, size))
		{}
	};
}