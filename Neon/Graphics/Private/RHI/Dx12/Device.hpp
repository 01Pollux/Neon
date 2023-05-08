#pragma once

#include <RHI/Device.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12RenderDevice final : public IRenderDevice
    {
    public:
        Dx12RenderDevice();

        /// <summary>
        /// Gets the global render device.
        /// </summary>
        [[nodiscard]] static Dx12RenderDevice* Get();

        /// <summary>
        /// Get dxgi factory.
        /// </summary>
        [[nodiscard]] IDXGIFactory* GetDxgiFactory();

        /// <summary>
        /// Get dx12 device.
        /// </summary>
        [[nodiscard]] ID3D12Device* GetDevice();

    private:
        /// <summary>
        /// Enable debug layer if needed.
        /// </summary>
        void EnableDebugLayerIfNeeded();

        /// <summary>
        /// Create dxgi factory.
        /// </summary>
        void CreateFactory();

        /// <summary>
        /// Create dx12 device.
        /// </summary>
        void CreateDevice();

        /// <summary>
        /// Get best adapter for render device.
        /// </summary>
        Win32::ComPtr<IDXGIAdapter> GetBestAdapter() const;

    private:
        Win32::ComPtr<IDXGIFactory> m_DxgiFactory;
        Win32::ComPtr<ID3D12Device> m_Device;
    };
} // namespace Neon::RHI