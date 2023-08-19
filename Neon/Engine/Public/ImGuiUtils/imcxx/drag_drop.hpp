#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Drag and Drop
	/// - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload() + EndDragDropSource().
	/// - On target candidates, call BeginDragDropTarget(), if it returns true also call AcceptDragDropPayload() + EndDragDropTarget().
	/// - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip, see #1725)
	/// - An item can be both drag source and drop target.
	/// </summary>
	class [[nodiscard]] drag_drop_source : public scope_wrap<drag_drop_source>
	{
		friend class scope_wrap<drag_drop_source>;
	public:
		class item;

		drag_drop_source(ImGuiDragDropFlags flags = 0) :
			scope_wrap(ImGui::BeginDragDropSource(flags))
		{}

		drag_drop_source(const char* type, const void* data, size_t sz, ImGuiCond cond = 0, ImGuiDragDropFlags flags = 0) :
			drag_drop_source(flags)
		{
			if (*this)
			{
				set_data(type, data, sz, cond);
			}
		}

		/// <summary>
		/// type is a user defined string of maximum 32 characters.
		/// Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui. 
		/// Return true when payload has been accepted.
		/// </summary>
		bool set_data(const char* type, const void* data, size_t sz, ImGuiCond cond = 0)
		{
			return ImGui::SetDragDropPayload(type, data, sz, cond);
		}

		/// <summary>
		/// type is a user defined string of maximum 32 characters.
		/// Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui. 
		/// Return true when payload has been accepted.
		/// </summary>
		template<typename _Ty>
		bool set_data(const char* type, const _Ty& data, ImGuiCond cond = 0)
		{
			return ImGui::SetDragDropPayload(type, std::addressof(data), sizeof(data), cond);
		}

	private:
		void destruct()
		{
			ImGui::EndDragDropSource();
		}
	};

	/// <summary>
	/// Drag and Drop
	/// - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload() + EndDragDropSource().
	/// - On target candidates, call BeginDragDropTarget(), if it returns true also call AcceptDragDropPayload() + EndDragDropTarget().
	/// - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip, see #1725)
	/// - An item can be both drag source and drop target.
	/// </summary>
	class [[nodiscard]] drag_drop_target : public scope_wrap<drag_drop_target>
	{
		friend class scope_wrap<drag_drop_target>;
	public:
		class item;

		drag_drop_target() :
			scope_wrap(ImGui::BeginDragDropTarget())
		{}

		drag_drop_target(const ImGuiPayload*& out_data, const char* type, ImGuiDragDropFlags flags = 0) :
			drag_drop_target()
		{
			if (*this)
				out_data = get_data(type, flags);
		}

		/// <summary>
		/// accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
		/// </summary>
		const ImGuiPayload* get_data(const char* type, ImGuiDragDropFlags flags = 0)
		{
			return ImGui::AcceptDragDropPayload(type, flags);
		}
		
		/// <summary>
		/// peek directly into the current payload from anywhere. may return NULL. use ImGuiPayload::IsDataType() to test for the payload type.
		/// </summary>
		const ImGuiPayload* get_data()
		{
			return ImGui::GetDragDropPayload();
		}

	private:
		void destruct()
		{
			ImGui::EndDragDropTarget();
		}
	};
}
