#pragma once

#include <array>
#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Tables
	/// - Full-featured replacement for old Columns API.
	/// - See Demo->Tables for demo code.
	/// - See top of imgui_tables.cpp for general commentary.
	/// - See ImGuiTableFlags_ and ImGuiTableColumnFlags_ enums for a description of available flags.
	/// The typical call flow is:
	/// - 1. Call BeginTable().
	/// - 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
	/// - 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of columns/rows.
	/// - 4. Optionally call TableHeadersRow() to submit a header row. Names are pulled from TableSetupColumn() data.
	/// - 5. Populate contents:
	///    - In most situations you can use TableNextRow() + TableSetColumnIndex(N) to start appending into a column.
	///    - If you are using tables as a sort of grid, where every columns is holding the same type of contents,
	///      you may prefer using TableNextColumn() instead of TableNextRow() + TableSetColumnIndex().
	///      TableNextColumn() will automatically wrap-around into the next row if needed.
	///    - IMPORTANT: Comparatively to the old Columns() API, we need to call TableNextColumn() for the first column!
	///    - Summary of possible call flow:
	///        --------------------------------------------------------------------------------------------------------
	///        TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
	///        TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
	///                          TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
	///        TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
	///        --------------------------------------------------------------------------------------------------------
	/// - 5. Call EndTable()
	/// </summary>
	class [[nodiscard]] table : public scope_wrap<table>
	{
		friend class scope_wrap<table>;
	public:
		class column;

		template<typename _StrTy>
		table(const _StrTy& name, int column, ImGuiTableFlags flags = 0, const ImVec2& outer_size = {}, float inner_width = 0.0f) :
			scope_wrap(ImGui::BeginTable(impl::get_string(name), column, flags, outer_size, inner_width))
		{}

		struct setup_no_row {};
		struct setup_info
		{
			const char* label;
			ImGuiTableColumnFlags flags = 0;
			float init_width_or_weight = 0.f;
			ImGuiID user_id = 0;
		};

		/// <summary>
		/// Tables: Headers & Columns declaration
		/// - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
		/// - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
		///   Headers are required to perform: reordering, sorting, and opening the context menu.
		///   The context menu can also be made available in columns body using ImGuiTableFlags_ContextMenuInBody.
		/// - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
		///   some advanced use cases (e.g. adding custom widgets in header row).
		/// - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
		/// 
		/// _Args can be either a c-string, 'std::string', 'std::string_view' or 'setup_info'
		/// </summary>
		template<typename ..._Args>
		void setup(setup_no_row, const _Args&... args)
		{
			table_setup_impl(args...);
			ImGui::TableHeadersRow();
		}
		
		/// <summary>
		/// Tables: Headers & Columns declaration
		/// - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
		/// - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
		///   Headers are required to perform: reordering, sorting, and opening the context menu.
		///   The context menu can also be made available in columns body using ImGuiTableFlags_ContextMenuInBody.
		/// - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
		///   some advanced use cases (e.g. adding custom widgets in header row).
		/// - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
		/// 
		/// _Args can be either a c-string, 'std::string', 'std::string_view' or 'setup_info'
		/// </summary>
		template<typename ..._Args>
		void setup(int cols, int rows, const _Args&... args)
		{
			ImGui::TableSetupScrollFreeze(cols, rows);
			table_setup_impl(args...);
			ImGui::TableHeadersRow();
		}


		void next_row(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f)
		{
			ImGui::TableNextRow(row_flags, min_row_height);
		}

		/// <summary>
		/// calls next_column() and invoke the function if the former returned true
		/// </summary>
		template<typename _InvokeTy, typename ..._Args>
		void nextrow_and_invoke(_InvokeTy&& fn, _Args&&... args) noexcept(std::is_nothrow_invocable_v<_InvokeTy, _Args...>)
		{
			next_row();
			fn(std::forward<_Args>(args)...);
		}

		bool next_column()
		{
			return ImGui::TableNextColumn();
		}

		/// <summary>
		/// calls next_column() and invoke the function if the former returned true
		/// </summary>
		template<typename _InvokeTy, typename ..._Args>
		void nextcol_and_invoke(_InvokeTy&& fn, _Args&&... args) noexcept(std::is_nothrow_invocable_v<_InvokeTy, _Args...>)
		{
			if (next_column())
				fn(std::forward<_Args>(args)...);
		}

		bool set_column(int column_n)
		{
			return ImGui::TableSetColumnIndex(column_n);
		}

		[[nodiscard]] column current_column() const;

		[[nodiscard]] column get_column(int column_n) const;
		
		/// <summary>
		/// change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details
		/// </summary>
		void set_color(ImGuiTableBgTarget target, ImU32 color, int column_n = -1)
		{
			ImGui::TableSetBgColor(target, color, column_n);
		}

		/// <summary>
		/// change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details
		/// </summary>
		void set_color(ImGuiTableBgTarget target, const ImVec4& color, int column_n = -1)
		{
			set_color(target, ImGui::ColorConvertFloat4ToU32(color), column_n);
		}

		/// <summary>
		/// change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details
		/// </summary>
		void set_color(ImGuiTableBgTarget target, const std::array<float, 4>& color, int column_n = -1)
		{
			set_color(target, ImVec4{ color[0], color[1], color[2], color[3] }, column_n);
		}

		/// <summary>
		///  Tables: Sorting
		/// - Call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
		/// - When 'SpecsDirty == true' you should sort your data. It will be true when sorting specs have changed
		///   since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting, else you may
		///   wastefully sort your data every frame!
		/// - Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
		/// </summary>
		[[nodiscard]] ImGuiTableSortSpecs* specs() const
		{
			return ImGui::TableGetSortSpecs();
		}

	private:
		template<typename _Ty, typename ..._Args>
		void table_setup_impl(const _Ty& arg, const _Args&... rest)
		{
			if constexpr (std::is_same_v<setup_info, _Ty>)
				ImGui::TableSetupColumn(arg.label, arg.flags, arg.init_width_or_weight, arg.user_id);
			else
				ImGui::TableSetupColumn(impl::get_string(arg));

			if constexpr (sizeof...(_Args) > 0)
				table_setup_impl(rest...);
		}

		void destruct()
		{
			ImGui::EndTable();
		}
	};

	class [[nodiscard]] table::column
	{
	public:
		column(int column) noexcept : m_Column(column) 
		{}

		/// <summary>
		/// get the index of the column
		/// </summary>
		[[nodiscard]] int index() const noexcept
		{
			return m_Column;
		}

		/// <summary>
		/// get the name of the column
		/// </summary>
		[[nodiscard]] const char* name() const
		{
			return ImGui::TableGetColumnName(index());
		}
		
		/// <summary>
		/// get the flags of the column
		/// </summary>
		[[nodiscard]] ImGuiTableFlags flags() const
		{
			return ImGui::TableGetColumnFlags(index());
		}

		/// <summary>
		/// change user accessible enabled/disabled state of a column. Set to false to hide the column. 
		/// User can use the context menu to change this themselves (right-click in headers, or right-click in columns body with ImGuiTableFlags_ContextMenuInBody)
		/// </summary>
		[[nodiscard]] void set_state(bool enable) const
		{
			ImGui::TableSetColumnEnabled(index(), enable);
		}

		/// <summary>
		/// change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details
		/// </summary>
		void set_color(ImU32 color)
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color, index());
		}

		/// <summary>
		/// change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details
		/// </summary>
		void set_color(const ImVec4& color)
		{
			set_color(ImGui::ColorConvertFloat4ToU32(color));
		}

		/// <summary>
		/// change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details
		/// </summary>
		void set_color(const std::array<float, 4>& color)
		{
			set_color(ImVec4{ color[0], color[1], color[2], color[3] });
		}

	private:
		int m_Column;
	};


	inline auto imcxx::table::get_column(int column_idx) const -> column
	{
		return column{ column_idx };
	}

	inline auto imcxx::table::current_column() const -> column
	{
		return get_column(ImGui::TableGetColumnIndex());
	}
}
