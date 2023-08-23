#pragma once

#include <type_traits>
#include <numeric>
#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Widgets: Drag Sliders
	/// - CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use ImGuiSliderFlags_AlwaysClamp to always clamp.
	/// - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every functions, note that a 'float v[X]' function argument is the same as 'float* v',
	///   the array syntax is just a way to document the number of elements that are expected to be accessible. You can pass address of your first element out of a contiguous set, e.g. &myvector.x
	/// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
	/// - Format string may also be set to NULL or use the default format ("%f" or "%d").
	/// - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
	/// - Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits if ImGuiSliderFlags_AlwaysClamp is not used.
	/// - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
	/// - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
	/// - Legacy: Pre-1.78 there are DragXXX() function signatures that takes a final `float power=1.0f' argument instead of the `ImGuiSliderFlags flags=0' argument.
	///   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
	/// </summary>
	class drag : public scope_wrap<drag, scope_traits::no_dtor>
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
		/// Calls 'ImGui::DragScalar'
		/// </summary>
		template<typename _StrTy, typename _Ty>
		drag(
			const _StrTy& label,
			_Ty* v,
			float speed = 1.f,
			_Ty v_min = (std::numeric_limits<_Ty>::min)(), 
			_Ty v_max = (std::numeric_limits<_Ty>::max)(),
			const char* format = impl::default_c_format<_Ty>, 
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::DragScalar()'");
#if _HAS_CXX20
			m_Result._Value = ImGui::DragScalar(
				impl::get_string(label),
				type,
				std::bit_cast<void*>(v),
				speed,
				std::bit_cast<const void*>(&v_min),
				std::bit_cast<const void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::DragScalar(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v),
				speed,
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
		}

		/// <summary>
		/// Calls 'ImGui::DragScalar'
		/// </summary>
		template<typename _StrTy, typename _Ty, typename = std::enable_if_t<!std::is_array_v<_Ty>>>
		drag(
			const _StrTy& label,
			_Ty& v,
			float speed = 1.f,
			_Ty v_min = (std::numeric_limits<_Ty>::min)(),
			_Ty v_max = (std::numeric_limits<_Ty>::max)(),
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		) :
			drag(label, &v, speed, v_min, v_max, format, flags)
		{}


		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v', where v is contiguous-pointer
		/// </summary>
		template<typename _StrTy, typename _Ty>
		drag(
			pointer,
			const _StrTy& label,
			_Ty* v,
			size_t size,
			float speed = 1.f,
			_Ty v_min = (std::numeric_limits<_Ty>::min)(),
			_Ty v_max = (std::numeric_limits<_Ty>::max)(),
			const char* format = impl::default_c_format<_Ty>, 
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::DragScalarN()'");
#if _HAS_CXX20
			m_Result._Value = ImGui::DragScalarN(
				impl::get_string(label),
				type,
				std::bit_cast<void*>(v),
				size,
				speed,
				std::bit_cast<const void*>(&v_min),
				std::bit_cast<const void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::DragScalarN(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v), 
				size,
				speed,
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
}

		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy, typename _Ty, size_t _Size>
		drag(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			std::array<_Ty, _Size>* v, 
			float speed = 1.f,
			_Ty v_min = (std::numeric_limits<_Ty>::min)(),
			_Ty v_max = (std::numeric_limits<_Ty>::max)(), 
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::DragScalarN()'");
			static constexpr size_t size = _ViewSize ? _ViewSize : _Size;
#if _HAS_CXX20
			m_Result._Value = ImGui::DragScalarN(
				impl::get_string(label),
				type,
				std::bit_cast<void*>(v->data()),
				size,
				speed,
				std::bit_cast<void*>(&v_min),
				std::bit_cast<void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::DragScalarN(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v->data()),
				size,
				speed,
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
		}

		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy, typename _Ty, size_t _Size>
		drag(
			array<_ViewSize> arr_size, 
			const _StrTy& label,
			_Ty(&v)[_Size], 
			float speed = 1.f,
			_Ty v_min = (std::numeric_limits<_Ty>::min)(),
			_Ty v_max = (std::numeric_limits<_Ty>::max)(),
			const char* format = impl::default_c_format<_Ty>,
			ImGuiSliderFlags flags = 0
		)
		{
			static constexpr ImGuiDataType type = impl::to_imdatatype<_Ty>();
			static_assert(type != ImGuiDataType_COUNT, "Invalid type was passed to 'ImGui::DragScalarN()'");
			static constexpr size_t size = _ViewSize ? _ViewSize : _Size;
#if _HAS_CXX20
			m_Result._Value = ImGui::DragScalarN(
				impl::get_string(label),
				type,
				std::bit_cast<void*>(static_cast<_Ty*>(v)),
				size,
				speed,
				std::bit_cast<void*>(&v_min),
				std::bit_cast<void*>(&v_max),
				format,
				flags
			);
#else
			m_Result._Value = ImGui::DragScalarN(
				impl::get_string(label),
				type,
				reinterpret_cast<void*>(v),
				size,
				speed,
				reinterpret_cast<const void*>(&v_min),
				reinterpret_cast<const void*>(&v_max),
				format,
				flags
			);
#endif
		}

		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy, typename _Ty, size_t _Size>
		drag(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			std::array<_Ty, _Size>& v, 
			float speed = 1.f,
			_Ty v_min = (std::numeric_limits<_Ty>::min)(),
			_Ty v_max = (std::numeric_limits<_Ty>::max)(),
			const char* format = impl::default_c_format<_Ty>, 
			ImGuiSliderFlags flags = 0
		) :
			drag(arr_size, label, &v, speed, v_min, v_max, format, flags)
		{}


		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v', overload for 'ImVec2'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		drag(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec2* v, 
			float speed = 1.f,
			float v_min = (std::numeric_limits<float>::min)(),
			float v_max = (std::numeric_limits<float>::max)(),
			const char* format = "%.3f",
			ImGuiSliderFlags flags = 0
		) :
#if _HAS_CXX20
			drag(arr_size, label, std::bit_cast<std::array<float, 2>*>(&v), speed, v_min, v_max, format, flags)
#else
			drag(arr_size, label, reinterpret_cast<std::array<float, 2>*>(&v), speed, v_min, v_max, format, flags)
#endif
		{}


		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v', overload for 'ImVec2'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		drag(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec2& v,
			float speed = 1.f,
			float v_min = (std::numeric_limits<float>::min)(),
			float v_max = (std::numeric_limits<float>::max)(),
			const char* format = "%.3f", 
			ImGuiSliderFlags flags = 0
		) :
			drag(arr_size, label, &v, speed, v_min, v_max, format, flags)
		{}


		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v', overload for 'ImVec4'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		drag(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec4* v,
			float speed = 1.f,
			float v_min = (std::numeric_limits<float>::min)(),
			float v_max = (std::numeric_limits<float>::max)(),
			const char* format = "%.3f", 
			ImGuiSliderFlags flags = 0
		) :
#if _HAS_CXX20
			drag(arr_size, label, std::bit_cast<std::array<float, 4>*>(&v), speed, v_min, v_max, format, flags)
#else
			drag(arr_size, label, reinterpret_cast<std::array<float, 4>*>(&v), speed, v_min, v_max, format, flags)
#endif
		{}

		/// <summary>
		/// Calls 'ImGui::DragScalarN' with a smaller or equal to array size 'v', overload for 'ImVec4'
		/// </summary>
		template<size_t _ViewSize, typename _StrTy>
		drag(
			array<_ViewSize> arr_size,
			const _StrTy& label,
			ImVec4& v,
			float speed = 1.f,
			float v_min = (std::numeric_limits<float>::min)(),
			float v_max = (std::numeric_limits<float>::max)(),
			const char* format = "%.3f",
			ImGuiSliderFlags flags = 0
		) :
			drag(arr_size, label, &v, speed, v_min, v_max, format, flags)
		{}
	};
}