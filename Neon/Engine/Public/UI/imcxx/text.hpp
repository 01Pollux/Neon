#pragma once

#include "scopes.hpp"
#include "imgui/imgui_internal.h"

namespace imcxx
{
	/// <summary>
	/// Widgets: Text
	/// </summary>
	class text : public scope_wrap<text, scope_traits::no_dtor, false>
	{
	public:
		class no_format {};
		class disabled {};
		class wrapped {};
		class label {};
		class bullet {};

		/// <summary>
		///  text without formatting. Roughly equivalent to Text("%s", text) but: A) doesn't require null terminated string if 'text_end' is specified, B) it's faster, no memory copy is done, no buffer size limits, recommended for long chunks of text.
		/// </summary>
		text(no_format, const char* begin, const char* end = nullptr)
		{
			ImGui::TextUnformatted(begin, end);
		}

		template<size_t _Size>
		text(const char (&str)[_Size]) :
			text(no_format{}, str, str + _Size)
		{}

		/// <summary>
		///  text without formatting. Roughly equivalent to Text("%s", text) but: A) doesn't require null terminated string if 'text_end' is specified, B) it's faster, no memory copy is done, no buffer size limits, recommended for long chunks of text.
		/// </summary>
		text(std::string_view str) : 
			text(no_format{}, str.data(), str.data() + str.size())
		{}

		/// <summary>
		/// formatted text
		/// </summary>
		text(const char* fmt, ...) IM_FMTLIST(1)
		{
			va_list args;
			va_start(args, fmt);
			ImGui::TextV(fmt, args);
			va_end(args);
		}


		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
		/// </summary>
		text(const ImVec4& color, no_format, const char* begin, const char* end)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted(begin, end);
			ImGui::PopStyleColor();
		}

		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
		/// </summary>
		template<size_t _Size>
		text(const ImVec4& color, const char (&str)[_Size]) :
			text(color, no_format{}, str, str + _Size)
		{}

		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
		/// </summary>
		text(const ImVec4& color, std::string_view str) :
			text(color, no_format{}, str.data(), str.data() + str.size())
		{}

		/// <summary>
		/// shortcut for TextColoredV();
		/// </summary>
		text(const ImVec4& color, const char* fmt, ...) IM_FMTLIST(2)
		{
			va_list args;
			va_start(args, fmt);
			ImGui::TextColoredV(color, fmt, args);
			va_end(args);
		}


		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
		/// </summary>
		text(disabled, no_format, const char* begin, const char* end = nullptr) :
			text(ImGui::GetStyle().Colors[ImGuiCol_::ImGuiCol_TextDisabled], begin, end)
		{}

		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
		/// </summary>
		template<size_t _Size>
		text(disabled, const char(&str)[_Size]) :
			text(disabled{}, no_format{}, str, str + _Size)
		{}

		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
		/// </summary>
		text(disabled, std::string_view str) :
			text(disabled{}, no_format{}, str.data(), str.data() + str.size())
		{}

		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
		/// </summary>
		text(disabled, const char* fmt, ...) IM_FMTLIST(2)
		{
			va_list args;
			va_start(args, fmt);
			ImGui::TextDisabledV(fmt, args);
			va_end(args);
		}


		/// <summary>
		/// shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
		/// </summary>
		text(wrapped, no_format, const char* begin, const char* end = nullptr)
		{
			ImGuiContext& g = *GImGui;
			bool need_backup = (ImGui::GetCurrentWindow()->DC.TextWrapPos < 0.0f);  // Keep existing wrap position if one is already set
			if (need_backup)
				ImGui::PushTextWrapPos(0.f);
			ImGui::TextUnformatted(begin, end);
			if (need_backup)
				ImGui::PopTextWrapPos();
		}

		/// <summary>
		/// shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
		/// </summary>
		template<size_t _Size>
		text(wrapped, const char (&str)[_Size]) :
			text(wrapped{}, no_format{}, str, str + _Size)
		{}

		/// <summary>
		/// shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
		/// </summary>
		text(wrapped wrap_state, std::string_view str) :
			text(wrap_state, no_format{}, str.data(), str.data() + str.size())
		{}

		/// <summary>
		/// shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
		/// </summary>
		text(wrapped, const char* fmt, ...) IM_FMTLIST(2)
		{
			va_list args;
			va_start(args, fmt);
			ImGui::TextWrappedV(fmt, args);
			va_end(args);
		}


		/// <summary>
		/// display text+label aligned the same way as value+label widgets
		/// </summary>
		text(label, const char* label, const char* fmt, ...) IM_FMTLIST(2)
		{
			va_list args;
			va_start(args, fmt);
			ImGui::LabelTextV(label, fmt, args);
			va_end(args);
		}

		/// <summary>
		/// shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
		/// </summary>
		text(label label_state, std::string_view label, std::string_view str) :
			text(label_state, label.data(), str.data())
		{}


		/// <summary>
		/// display text+label aligned the same way as value+label widgets
		/// </summary>
		text(bullet, const char* fmt, ...) IM_FMTLIST(2)
		{
			va_list args;
			va_start(args, fmt);
			ImGui::BulletTextV(fmt, args);
			va_end(args);
		}

		/// <summary>
		/// shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
		/// </summary>
		text(bullet bullet_state, std::string_view str) :
			text(bullet_state, str.data())
		{}
	};
}