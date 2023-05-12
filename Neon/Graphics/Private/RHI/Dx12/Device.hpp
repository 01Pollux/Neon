#pragma once

#include <RHI/Device.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>

namespace Neon::RHI
{
    class Dx12RenderDevice final : public IRenderDevice
    {
    public:
        Dx12RenderDevice();

        [[nodiscard]] IResourceStateManager* GetStateManager() override;

    public:
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

        /// <summary>
        /// Get descriptor size.
        /// </summary>
        uint32_t GetDescriptorSize(
            D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

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
        /// <summary>
        /// Fill in descriptor sizes.
        /// </summary>
        void FillInDescriptorSizes();

    private:
        struct HeapDescriptorSizeType
        {
            uint16_t DSV = 0;
            uint16_t RTV = 0;
            uint16_t SAM = 0;
            union {
                uint16_t CBV = 0;
                uint16_t SRV;
                uint16_t UAV;
            };
        };

    private:
        Win32::ComPtr<IDXGIFactory> m_DxgiFactory;
        Win32::ComPtr<ID3D12Device> m_Device;

        Dx12ResourceStateManager m_StateManager;
        HeapDescriptorSizeType   m_HeapDescriptorSize;
    };
} // namespace Neon::RHI