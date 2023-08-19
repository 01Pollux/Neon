#pragma once

#include "scopes.hpp"

// TODO: Add color_view ? and add other colors thing to have same logic instead of copy_paste default_c_format
namespace imcxx
{
	/// <summary>
	/// Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
	/// - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible.
	/// - You can pass the address of a first float element out of a contiguous structure, e.g. &myvector.x
	/// </summary>
	class color : public scope_wrap<color, scope_traits::no_dtor>
	{
	public:
		struct edit {};
		struct picker {};

		template<typename _StrTy, typename _Ty>
		color(picker, const _StrTy& label, _Ty& col, ImGuiColorEditFlags flags = 0, const float* ref_col = nullptr)
		{
			using inner_type = impl::array_value_t<_Ty>;
			constexpr float to_ratio = impl::is_imvec4_v<_Ty> || std::is_floating_point_v<inner_type> ? 1.f : 255.f;
			static constexpr size_t size = impl::array_value_size<_Ty>;

			float tmp[4];
			if constexpr (impl::is_imvec4_v<_Ty>)
			{
				tmp[0] = col.x;
				tmp[1] = col.y;
				tmp[2] = col.z;
				tmp[3] = col.w;
			}
			else
			{
				tmp[0] = col[0] / to_ratio;
				tmp[1] = col[1] / to_ratio;
				tmp[2] = col[2] / to_ratio;
				tmp[3] = size < 4 ? 1.f : col[3] / to_ratio;
			}

			if (size < 4)
				flags |= ImGuiColorEditFlags_NoAlpha;
			if (m_Result._Value = ImGui::ColorPicker4(impl::get_string(label), tmp, flags, ref_col))
			{
				if constexpr (impl::is_imvec4_v<_Ty>)
				{
					col.x = tmp[0] * to_ratio;
					col.y = tmp[1] * to_ratio;
					col.z = tmp[2] * to_ratio;
					col.w = tmp[3] * to_ratio;
				}
				else
				{
					for (size_t i = 0; i < size; i++)
						col[i] = static_cast<inner_type>(tmp[i] * to_ratio);
				}
			}
		}


		template<typename _StrTy, typename _Ty>
		color(edit, const _StrTy& label, _Ty& col, ImGuiColorEditFlags flags = 0)
		{
			using inner_type = impl::array_value_t<_Ty>;
			constexpr float to_ratio = impl::is_imvec4_v<_Ty> || std::is_floating_point_v<inner_type> ? 1.f : 255.f;
			static constexpr size_t size = impl::array_value_size<_Ty>;

			float tmp[4];
			if constexpr (impl::is_imvec4_v<_Ty>)
			{
				tmp[0] = col.x;
				tmp[1] = col.y;
				tmp[2] = col.z;
				tmp[3] = col.w;
			}
			else
			{
				tmp[0] = col[0] / to_ratio;
				tmp[1] = col[1] / to_ratio;
				tmp[2] = col[2] / to_ratio;
				tmp[3] = size < 4 ? 1.f : col[3] / to_ratio;
			}

			if (size < 4)
				flags |= ImGuiColorEditFlags_NoAlpha;
			if (m_Result._Value = ImGui::ColorEdit4(impl::get_string(label), tmp, flags))
			{
				if constexpr (impl::is_imvec4_v<_Ty>)
				{
					col.x = tmp[0] * to_ratio;
					col.y = tmp[1] * to_ratio;
					col.z = tmp[2] * to_ratio;
					col.w = tmp[3] * to_ratio;
				}
				else
				{
					for (size_t i = 0; i < size; i++)
						col[i] = static_cast<inner_type>(tmp[i] * to_ratio);
				}
			}
		}

		color(ImGuiColorEditFlags flags)
		{
			ImGui::SetColorEditOptions(flags);
		}
	};
}
