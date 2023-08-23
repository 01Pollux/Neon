#pragma once

#include "scopes.hpp"
#include <algorithm>

namespace imcxx
{
	class checkbox : public scope_wrap<checkbox, scope_traits::no_dtor>
	{
	public:
		template<typename _StrTy>
		checkbox(const _StrTy& label, bool* p_open) :
			scope_wrap(ImGui::Checkbox(impl::get_string(label), p_open))
		{}

		template<typename _StrTy>
		checkbox(const _StrTy& label, bool& r_open) :
			checkbox(label, &r_open)
		{}
		

		template<typename _StrTy, typename _Ty, typename _FlagsTy, typename = std::enable_if_t<std::is_integral_v<_Ty>>>
		checkbox(const _StrTy& label, _Ty* flags, _FlagsTy flag) :
			scope_wrap(ImGui::CheckboxFlagsT(impl::get_string(label), flags, static_cast<_Ty>(flag)))
		{}

		template<typename _StrTy, typename _Ty, typename _FlagsTy>
		checkbox(const _StrTy& label, _Ty& flags, _FlagsTy flag) :
			checkbox(label, &flags, flag)
		{}
	};


	
	template<size_t _Size>
	class checkbox_list
	{
	public:
		size_t size() const noexcept
		{
			return _Size;
		}

		checkbox& operator[](size_t index) const noexcept
		{
			return m_Checkboxes[index];
		}
		
		checkbox& operator[](size_t index) noexcept
		{
			return m_Checkboxes[index];
		}

		const checkbox* data() const noexcept
		{
			return m_Checkboxes;
		}

		checkbox* data() noexcept
		{
			return m_Checkboxes;
		}

		auto begin() const noexcept
		{
			return std::begin(m_Checkboxes);
		}
		
		auto end() const noexcept
		{
			return std::end(m_Checkboxes);
		}
		
		auto begin() noexcept
		{
			return std::begin(m_Checkboxes);
		}
		
		auto end() noexcept
		{
			return std::end(m_Checkboxes);
		}
		
		auto rbegin() const noexcept
		{
			return std::rbegin(m_Checkboxes);
		}
		
		auto rend() const noexcept
		{
			return std::rend(m_Checkboxes);
		}
		
		auto rbegin() noexcept
		{
			return std::rbegin(m_Checkboxes);
		}
		
		auto rend() noexcept
		{
			return std::rend(m_Checkboxes);
		}

		checkbox m_Checkboxes[_Size];
	};

	template<typename _Ty, typename ..._Args>
	checkbox_list(_Ty, _Args...) -> checkbox_list<1 + sizeof...(_Args)>;
}
