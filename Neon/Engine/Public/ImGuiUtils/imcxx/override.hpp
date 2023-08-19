#pragma once

#include "scopes.hpp"

namespace imcxx
{
	enum class shared_override_strategy
	{
		font, // ImGui::PushFont

		style_color, // ImGui::PushStyleColor
		style_var, // ImGui::PushStyleVar

		text_wrap, // ImGui::PushTextWrapPos
		item_flag, // ImGui::PushItemFlag
		item_width, // ImGui::PushItemWidth
		item_id, // ImGui::PushID

		focus_scope, // ImGui::PushFocusScope
		clip_rect, // ImGui::PushClipRect
		column_clip_rect, // ImGui::PushColumnClipRect
	};

	/// <summary>
	/// Useful for handling multiple Push/Pop of different types (check 'shared_override_strategy') when the items is in current window,
	/// </summary>
	template<shared_override_strategy _Strategy>
	class [[nodiscard]] shared_override
	{
	public:
		shared_override() = default;

		shared_override(const shared_override&) = delete;
		shared_override& operator=(const shared_override&) = delete;

		shared_override(shared_override&& other) noexcept :
			m_PopCount(std::exchange(other.m_PopCount, 0))
		{}
		shared_override& operator=(shared_override&& other) noexcept
		{
			if (this != &other)
			{
				m_PopCount += other.m_PopCount;
				other.m_PopCount = 0;
			}
			return *this;
		}

		template<typename ..._Args>
		shared_override(_Args&&... args)
		{
			push(std::forward<_Args>(args)...);
		}
		
		~shared_override()
		{
			if (m_PopCount)
				pop(m_PopCount);
		}

		template<typename ..._Args>
		void push(_Args&&... args)
		{
			static constexpr size_t strategy_size = get_strategy_size();

			if constexpr (strategy_size == 1)
				push_impl_1(std::forward<_Args>(args)...);
			else if constexpr (strategy_size == 2)
				push_impl_2(std::forward<_Args>(args)...);
			else if constexpr (strategy_size == 3)
				push_impl_3(std::forward<_Args>(args)...);
		}

		void unsafe_push(size_t count) noexcept
		{
			m_PopCount += count;
		}

		void pop(size_t count = 1)
		{
			IM_ASSERT(m_PopCount >= count);
			m_PopCount -= count;
			if constexpr (_Strategy == shared_override_strategy::font)
			{
				while (count-- != 0)
				{
					ImGui::PopFont();
				}
			}
			else if constexpr (_Strategy == shared_override_strategy::text_wrap)
			{
				while (count-- != 0)
				{
					ImGui::PopTextWrapPos();
				}
			}
			else if constexpr (_Strategy == shared_override_strategy::item_flag)
			{
				while (count-- != 0)
				{
					ImGui::PopItemFlag();
				}
			}
			else if constexpr (_Strategy == shared_override_strategy::item_width)
			{
				while (count-- != 0)
				{
					ImGui::PopItemWidth();
				}
			}
			else if constexpr (_Strategy == shared_override_strategy::item_id)
			{
				while (count-- != 0)
				{
					ImGui::PopID();
				}
			}
			else if constexpr (_Strategy == shared_override_strategy::focus_scope)
			{
				while (count-- != 0)
				{
					ImGui::PopFocusScope();
				}
			}
			else if constexpr (_Strategy == shared_override_strategy::clip_rect || _Strategy == shared_override_strategy::column_clip_rect)
			{
				while (count-- != 0)
				{
					ImGui::PopClipRect();
				}
			}
			else if constexpr (_Strategy == shared_override_strategy::style_color)
			{
				ImGui::PopStyleColor(static_cast<int>(count));
			}
			else if constexpr (_Strategy == shared_override_strategy::style_var)
			{
				ImGui::PopStyleVar(static_cast<int>(count));
			}
		}

		void pop_all()
		{
			pop(count());
		}

		void unsafe_pop(size_t count) noexcept
		{
			m_PopCount -= count;
		}

		[[nodiscard]] size_t count() const noexcept
		{
			return m_PopCount;
		}

	private:
		constexpr static size_t get_strategy_size()
		{
			switch (_Strategy)
			{
			case shared_override_strategy::clip_rect:
				return 3;

			case shared_override_strategy::style_color:
			case shared_override_strategy::style_var:
			case shared_override_strategy::item_flag:
				return 2;

			case shared_override_strategy::font:
			case shared_override_strategy::text_wrap:
			case shared_override_strategy::item_width:
			case shared_override_strategy::item_id:
			case shared_override_strategy::focus_scope:
			case shared_override_strategy::column_clip_rect:
				return 1;

			default:
			{
				return 0;
			}
			}
		}

		template<typename _Ty0, typename _Ty1, typename _Ty2, typename ..._TyX>
		void push_impl_3(const _Ty0& val0, const _Ty1& val1, _Ty2 val2, _TyX&&... others)
		{
			if constexpr (_Strategy == shared_override_strategy::clip_rect)
			{
				++m_PopCount;
				ImGui::PushClipRect(val0, val1, val2);
			}
			
			if constexpr (sizeof ...(_TyX) >= 3)
				push_impl_3(std::forward<_TyX>(others)...);
		}

		template<typename _Ty0, typename _Ty1, typename ..._TyX>
		void push_impl_2(_Ty0 type, const _Ty1& value, _TyX&&... others)
		{
			if constexpr (_Strategy == shared_override_strategy::style_color)
			{
				++m_PopCount;
				ImGui::PushStyleColor(type, value);
			}
			else if constexpr (_Strategy == shared_override_strategy::style_var)
			{
				++m_PopCount;
				ImGui::PushStyleVar(type, value);
			}
			else if constexpr (_Strategy == shared_override_strategy::item_flag)
			{
				++m_PopCount;
				ImGui::PushItemFlag(type, value);
			}

			if constexpr (sizeof ...(_TyX) >= 2)
				push_impl_2(std::forward<_TyX>(others)...);
		}

		template<typename _Ty0, typename ..._TyX>
		void push_impl_1(_Ty0 value, _TyX&&... others)
		{
			if constexpr (_Strategy == shared_override_strategy::font)
			{
				++m_PopCount;
				ImGui::PushFont(value);
			}
			else if constexpr (_Strategy == shared_override_strategy::text_wrap)
			{
				++m_PopCount;
				ImGui::PushTextWrapPos(value);
			}
			else if constexpr (_Strategy == shared_override_strategy::item_width)
			{
				ImGui::PushItemWidth(value);
			}
			else if constexpr (_Strategy == shared_override_strategy::item_id)
			{
				++m_PopCount;
				if constexpr (std::is_same_v<_Ty0, std::initializer_list<const char*>>)
					ImGui::PushID(*value.begin(), *value.end());
				else
					ImGui::PushID(value);
			}
			else if constexpr (_Strategy == shared_override_strategy::focus_scope)
			{
				++m_PopCount;
				ImGui::PushFocusScope(value);
			}
			else if constexpr (_Strategy == shared_override_strategy::column_clip_rect)
			{
				++m_PopCount;
				ImGui::PushColumnClipRect(value);
			}

			if constexpr (sizeof ...(_TyX) >= 1)
				push_impl_1(std::forward<_TyX>(others)...);
		}

	private:
		size_t m_PopCount{ };
	};

	using shared_font = shared_override<shared_override_strategy::font>;

	using shared_color = shared_override<shared_override_strategy::style_color>;
	using shared_style = shared_override<shared_override_strategy::style_var>;

	using shared_textwrap = shared_override<shared_override_strategy::text_wrap>;
	using shared_item_flag = shared_override<shared_override_strategy::item_flag>;
	using shared_item_width = shared_override<shared_override_strategy::item_width>;
	using shared_item_id = shared_override<shared_override_strategy::item_id>;

	using shared_focus_scope = shared_override<shared_override_strategy::focus_scope>;
	using shared_clip_rect = shared_override<shared_override_strategy::clip_rect>;
	using shared_column_rect = shared_override<shared_override_strategy::column_clip_rect>;
}