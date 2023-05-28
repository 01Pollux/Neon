#pragma once

#include <RHI/Device.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/ShaderCompiler.hpp>

namespace Neon::RHI
{
    class Dx12DeviceFeatures
    {
        friend class Dx12RenderDevice;

    public:
        /// <summary>
        /// Get root signature version.
        /// </summary>
        [[nodiscard]] D3D_ROOT_SIGNATURE_VERSION GetRootSignatureVersion() const;

        /// <summary>
        /// Get max view descriptor heap size for sampler or non-sampler.
        /// </summary>
        [[nodiscard]] uint32_t MaxDescriptorHeapSize(
            bool Sampler) const;

    private:
        void Initialize(
            ID3D12Device* Device);

    private:
        D3D_ROOT_SIGNATURE_VERSION m_RootSignatureVersion;
        uint32_t                   m_DescriptorHeapSize[2];
    };

    class Dx12RenderDevice final : public IRenderDevice
    {
    public:
        Dx12RenderDevice();
        ~Dx12RenderDevice() override;

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
        /// Get dx12 dxgi adapter.
        /// </summary>
        [[nodiscard]] IDXGIAdapter* GetAdapter();

        /// <summary>
        /// Get dx12 device.
        /// </summary>
        [[nodiscard]] ID3D12Device* GetDevice();

        /// <summary>
        /// Get descriptor size.
        /// </summary>
        [[nodiscard]] uint32_t GetDescriptorSize(
            D3D12_DESCRIPTOR_HEAP_TYPE Type) const;

        /// <summary>
        /// Get device features.
        /// </summary>
        [[nodiscard]] const Dx12DeviceFeatures& GetFeatures() const;

        /// <summary>
        /// Get shader compiler.
        /// </summary>
        [[nodiscard]] Dx12ShaderCompiler* GetShaderCompiler();

    private:
        /// <summary>
        /// Load pix runtime.
        /// </summary>
        void LoadPixRuntime();

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
        /// Check device features.
        /// </summary>
        void CheckDeviceFeatures();

        /// <summary>
        /// Get best adapter for render device.
        /// </summary>
        [[nodiscard]] Win32::ComPtr<IDXGIAdapter> GetBestAdapter() const;

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
        Win32::ComPtr<IDXGIAdapter> m_Adapter;
        Win32::ComPtr<ID3D12Device> m_Device;

        HeapDescriptorSizeType m_HeapDescriptorSize;
        Dx12DeviceFeatures     m_DeviceFeatures;

        Dx12ShaderCompiler m_Compiler;
    };
} // namespace Neon::RHI