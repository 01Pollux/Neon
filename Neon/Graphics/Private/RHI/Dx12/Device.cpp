#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    static inline std::mutex     s_CreateDeviceMutex;
    static inline uint16_t       s_NumDevices = 0;
    static inline IRenderDevice* s_Device     = nullptr;

    IRenderDevice* IRenderDevice::CreateGlobal()
    {
        std::lock_guard Lock(s_CreateDeviceMutex);
        if (!s_NumDevices++)
        {
            NEON_INFO_TAG("Graphics", "Creating DirectX 12 Render Device");
            s_Device = new Dx12RenderDevice();
        }
        return s_Device;
    }

    void IRenderDevice::DestroyGlobal()
    {
        std::lock_guard Lock(s_CreateDeviceMutex);
        if (!--s_NumDevices)
        {
            NEON_INFO_TAG("Graphics", "Destroying DirectX 12 Render Device");
            delete s_Device;
            s_Device = nullptr;
        }
    }

    IRenderDevice* IRenderDevice::Get()
    {
        return s_Device;
    }

    //

    Dx12RenderDevice::Dx12RenderDevice()
    {
        EnableDebugLayerIfNeeded();
        CreateFactory();
        CreateDevice();
    }

    Dx12RenderDevice* Dx12RenderDevice::Get()
    {
        return static_cast<Dx12RenderDevice*>(IRenderDevice::Get());
    }

    IDXGIFactory* Dx12RenderDevice::GetDxgiFactory()
    {
        return m_DxgiFactory.Get();
    }

    ID3D12Device* Dx12RenderDevice::GetDevice()
    {
        return m_Device.Get();
    }

    //

    void Dx12RenderDevice::EnableDebugLayerIfNeeded()
    {
        Win32::ComPtr<ID3D12Debug> DebugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)));
        DebugController->EnableDebugLayer();
    }

    void Dx12RenderDevice::CreateFactory()
    {
        ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_DxgiFactory)));
    }

    void Dx12RenderDevice::CreateDevice()
    {
        DXGI_ADAPTER_DESC Desc;
        auto              Adapter = GetBestAdapter();
        Adapter->GetDesc(&Desc);

        NEON_TRACE_TAG("Graphics", "Vendor: {:X}", Desc.VendorId);
        NEON_TRACE_TAG("Graphics", "Device: {:X}", Desc.DeviceId);
        NEON_TRACE_TAG("Graphics", "Revision: {}", Desc.Revision);
        NEON_TRACE_TAG("Graphics", "Dedicated Video Memory: {} Gb", Desc.DedicatedVideoMemory / (1024.f * 1024.f * 1024.f));
        NEON_TRACE_TAG("Graphics", "Dedicated System Memory: {} Gb", Desc.DedicatedSystemMemory / (1024.f * 1024.f * 1024.f));
        NEON_TRACE_TAG("Graphics", "Shared System Memory: {} Gb", Desc.SharedSystemMemory / (1024.f * 1024.f * 1024.f));
        NEON_TRACE_TAG("Graphics", "Description: {}", StringUtils::StringTransform<StringU8>(Desc.Description));

        ThrowIfFailed(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));
    }

    Win32::ComPtr<IDXGIAdapter> Dx12RenderDevice::GetBestAdapter() const
    {
        Win32::ComPtr<IDXGIAdapter> Adapter;
        Win32::ComPtr<IDXGIAdapter> BestAdapter;
        size_t                      BestVideoMemory = 0;
        for (UINT i = 0; m_DxgiFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            if (FAILED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                continue;
            }

            DXGI_ADAPTER_DESC Desc;
            Adapter->GetDesc(&Desc);
            if (Desc.DedicatedVideoMemory > BestVideoMemory)
            {
                BestAdapter     = Adapter;
                BestVideoMemory = Desc.DedicatedVideoMemory;
            }
        }
        return BestAdapter;
    }
} // namespace Neon::RHI