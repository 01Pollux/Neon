#pragma once

#include "scopes.hpp"

namespace imcxx
{
	/// <summary>
	/// Viewports
	/// - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
	/// - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
	/// - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
	/// </summary>
	class [[nodiscard]] viewport
	{
	public:
		viewport(ImGuiViewport* vp = ImGui::GetMainViewport()) :
			m_Viewport(vp)
		{}

		viewport(int id) :
			m_Viewport(ImGui::FindViewportByID(id))
		{}

		viewport(void* platform_hndl) :
			m_Viewport(ImGui::FindViewportByPlatformHandle(platform_hndl))
		{}

	public:
		[[nodiscard]] operator bool() const noexcept
		{
			return m_Viewport != nullptr;
		}

		[[nodiscard]] ImGuiViewport* get() const noexcept
		{
			return m_Viewport;
		}

		[[nodiscard]] ImGuiViewport* get() noexcept
		{
			return m_Viewport;
		}

		[[nodiscard]] const ImGuiViewport* operator->() const noexcept
		{
			return get();
		}

		[[nodiscard]] ImGuiViewport* operator->() noexcept
		{
			return get();
		}

	private:
		ImGuiViewport* m_Viewport;
	};
}