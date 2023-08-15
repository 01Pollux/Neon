#pragma once

namespace Neon::RHI
{
    class IGraphicsCommandList;
}

namespace Neon::RHI::ImGuiRHI
{
    /// <summary>
    /// Initialize ImGui.
    /// </summary>
    void InitializeImGui();

    /// <summary>
    /// Shutdown ImGui.
    /// </summary>
    void ShutdownImGui();

    /// <summary>
    /// Begin a new ImGui frame.
    /// </summary>
    void BeginImGuiFrame();

    /// <summary>
    /// End the current ImGui frame.
    /// </summary>
    void EndImGuiFrame(
        RHI::IGraphicsCommandList* CommandList);

    /// <summary>
    /// Set the default ImGui theme.
    /// </summary>
    void SetDefaultTheme();
} // namespace Neon::RHI::ImGuiRHI