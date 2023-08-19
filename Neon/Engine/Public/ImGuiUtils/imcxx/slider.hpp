#pragma once

#include <type_traits>
#include <numeric>
#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Widgets: Regular Sliders
	/// - CTRL+Click on any slider to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use ImGuiSliderFlags_AlwaysClamp to always clamp.
	/// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
	/// - Format string may also be set to NULL or use the default format ("%f" or "%d").
	/// - Legacy: Pre-1.78 there are SliderXXX() function signatures that takes a final `float power=1.0f' argument instead of the `ImGuiSliderFlags flags=0' argument.
	///   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361	
	/// </summary>
	class slider : public scope_wrap<slider, scope_traits::no_dtor>
	{
	public:
		struct angle {};
		struct pointer {};
		template<size_t _Size>
		struct array
		{
			static constexpr size_t size = _Size;
		};
		/// <summary>
		/// take the whole array instead of slicing it
		/// </summary>
		using auto_array = array<0>;

		/// <summary>
		/// Calls 'ImGui::SliderScalar'
		/// </summary>
		template<typename _StrTy, typename _Ty>
		slider(
			const _StrTy& label,
			_Ty* v,
			_Ty v_min,
			_Ty v_max,
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::SliderScalar()'");
#if _HAS_CXX20
			m_Result._Value = ImGui::SliderScalar(
				impl::get_string(label),
				type,
				std::bit_cast<void*>(v),
				std::bit_cast<const void*>(&v_min),
				std::bit_cast<const void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::SliderScalar(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v),
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
		}

		/// <summary>
		/// Calls 'ImGui::SliderScalar'
		/// </summary>
		template<typename _StrTy, typename _Ty, typename = std::enable_if_t<!std::is_array_v<_Ty>>>
		slider(
			const _StrTy& label,
			_Ty& v,
			_Ty v_min,
			_Ty v_max,
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		) :
			slider(label, &v, v_min, v_max, format, flags)
		{}


		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v', where v is contiguous-pointer
		/// </summary>
		template<typename _StrTy, typename _Ty>
		slider(
			pointer,
			const _StrTy& label,
			_Ty* v,
			size_t size,
			_Ty v_min,
			_Ty v_max,
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::SliderScalarN()'");
#if _HAS_CXX20
			m_Result._Value = ImGui::SliderScalarN(
				impl::get_string(label),
				type,
				std::bit_cast<void*>(v),
				size,
				std::bit_cast<const void*>(&v_min),
				std::bit_cast<const void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::SliderScalarN(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v), size,
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
		}


		/// <summary>
		/// Calls 'ImGui::SliderAngle' 
		/// </summary>
		template<typename _StrTy>
		slider(
			angle,
			const _StrTy& label,
			float* v_rad,
			float v_degrees_min = -360.0f,
			float v_degrees_max = +360.0f,
			const char* format = "%.0f deg",
			ImGuiSliderFlags flags = 0
		) :
			scope_wrap(ImGui::SliderAngle(impl::get_string(label), v_rad, v_degrees_min, v_degrees_max, format, flags))
		{}

		/// <summary>
		/// Calls 'ImGui::SliderAngle' 
		/// </summary>
		template<typename _StrTy>
		slider(
			angle,
			const _StrTy& label,
			float& v_rad,
			float v_degrees_min = -360.0f,
			float v_degrees_max = +360.0f,
			const char* format = "%.0f deg",
			ImGuiSliderFlags flags = 0
		) :
			scope_wrap(ImGui::SliderAngle(impl::get_string(label), &v_rad, v_degrees_min, v_degrees_max, format, flags))
		{}


		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy, typename _Ty, size_t _Size>
		slider(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			std::array<_Ty, _Size>* v,
			_Ty v_min,
			_Ty v_max,
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::SliderScalarN()'");
			static constexpr size_t size = _ViewSize ? _ViewSize : _Size;
#if _HAS_CXX20
			m_Result._Value = ImGui::SliderScalarN(
				impl::get_string(label),
				type,
				std::bit_cast<void*>(v->data()),
				size,
				std::bit_cast<void*>(&v_min),
				std::bit_cast<void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::SliderScalarN(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v->data()),
				size,
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
		}

		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy, typename _Ty, size_t _Size>
		slider(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			_Ty(&v)[_Size],
			_Ty v_min,
			_Ty v_max,
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::SliderScalarN()'");
			static constexpr size_t size = _ViewSize ? _ViewSize : _Size;
#if _HAS_CXX20
			m_Result._Value = ImGui::SliderScalarN(label,
				impl::get_string(label),
				std::bit_cast<void*>(static_cast<_Ty*>(v)),
				size,
				std::bit_cast<void*>(&v_min),
				std::bit_cast<void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::SliderScalarN(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v),
				size,
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
		}

		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy, typename _Ty, size_t _Size>
		slider(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			std::array<_Ty, _Size>& v,
			_Ty v_min,
			_Ty v_max,
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		) :
			slider(arr_size, label, &v, v_min, v_max, format, flags)
		{}


		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v', overload for 'ImVec2'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		slider(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec2* v,
			float v_min,
			float v_max,
			const char* format = "%.3f",
			ImGuiSliderFlags flags = 0
		) :
#if _HAS_CXX20
			slider(arr_size, label, std::bit_cast<std::array<float, 2>*>(&v), v_min, v_max, format, flags)
#else
			slider(arr_size, label, reinterpret_cast<std::array<float, 2>*>(&v), v_min, v_max, format, flags)
#endif
		{}


		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v', overload for 'ImVec2'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		slider(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec2& v,
			float v_min,
			float v_max,
			const char* format = "%.3f",
			ImGuiSliderFlags flags = 0
		) :
			slider(arr_size, label, &v, v_min, v_max, format, flags)
		{}


		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v', overload for 'ImVec4'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		slider(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec4* v,
			float v_min,
			float v_max,
			const char* format = "%.3f",
			ImGuiSliderFlags flags = 0
		) :
#if _HAS_CXX20
			slider(arr_size, label, std::bit_cast<std::array<float, 4>*>(&v), v_min, v_max, format, flags)
#else
			slider(arr_size, label, reinterpret_cast<std::array<float, 4>*>(&v), v_min, v_max, format, flags)
#endif
		{}

		/// <summary>
		/// Calls 'ImGui::SliderScalarN' with a smaller or equal to array size 'v', overload for 'ImVec4'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		slider(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec4& v,
			float v_min,
			float v_max,
			const char* format = "%.3f",
			ImGuiSliderFlags flags = 0
		) :
			slider(arr_size, label, &v, v_min, v_max, format, flags)
		{}
	};
}