#pragma once

#include <RHI/Device.hpp>
#include <Private/RHI/Dx12/Features.hpp>
#include <Private/RHI/Dx12/ShaderCompiler.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

namespace Neon::RHI
{
    class Dx12Texture;

    class Dx12RenderDevice final : public IRenderDevice
    {
    public:
        Dx12RenderDevice(
            const DeviceCreateDesc& DeviceDesc);
        NEON_CLASS_NO_COPYMOVE(Dx12RenderDevice);
        ~Dx12RenderDevice() override;

        RHI::ISwapchain* GetSwapchain() override;

    public:
        /// <summary>
        /// Gets the global render device.
        /// </summary>
        [[nodiscard]] static Dx12RenderDevice* Get();

        /// <summary>
        /// Initialize render device.
        /// </summary>
        void PostInitialize(
            Windowing::IWindowApp*     Window,
            const SwapchainCreateDesc& Swapchain);

        /// <summary>
        /// Shutdown render device.
        /// </summary>
        void Shudown();

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

        /// <summary>
        /// Get graphics memory allocator.
        /// </summary>
        [[nodiscard]] GraphicsMemoryAllocator* GetAllocator();

        /// <summary>
        /// Get resource state manager.
        /// </summary>
        [[nodiscard]] IResourceStateManager* GetStateManager();

        /// <summary>
        /// Get default texture.
        /// </summary>
        [[nodiscard]] const Ptr<ITexture>& GetDefaultTexture(
            DefaultTextures Type) const;

    private:
        /// <summary>
        /// Load pix runtime.
        /// </summary>
        void LoadPixRuntime();

        /// <summary>
        /// Enable debug layer if needed.
        /// </summary>
        void EnableDebugLayerIfNeeded(
            const DeviceCreateDesc& DeviceDesc);

        /// <summary>
        /// Create dxgi factory.
        /// </summary>
        void CreateFactory(
            const DeviceCreateDesc& DeviceDesc);

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
        [[nodiscard]] WinAPI::ComPtr<IDXGIAdapter> GetBestAdapter() const;

    private:
        /// <summary>
        /// Fill in descriptor sizes.
        /// </summary>
        void FillInDescriptorSizes();

        /// <summary>
        /// Create default textures.
        /// </summary>
        void CreateDefaultTextures();

    private:
        struct HeapDescriptorSizeType
        {
            uint32_t DSV = 0;
            uint32_t RTV = 0;
            uint32_t SAM = 0;
            union {
                uint32_t CBV = 0;
                uint32_t SRV;
                uint32_t UAV;
            };
        };

    private:
        WinAPI::ComPtr<IDXGIFactory> m_DxgiFactory;
        WinAPI::ComPtr<IDXGIAdapter> m_Adapter;
        WinAPI::ComPtr<ID3D12Device> m_Device;

        HeapDescriptorSizeType m_HeapDescriptorSize;
        Dx12DeviceFeatures     m_DeviceFeatures;

        Dx12ShaderCompiler m_Compiler;

        UPtr<GraphicsMemoryAllocator> m_MemoryAllocator;
        UPtr<Dx12Swapchain>           m_Swapchain;

        std::array<Ptr<ITexture>, size_t(DefaultTextures::Count)> m_DefaultTextures;
    };
} // namespace Neon::RHI