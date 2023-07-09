#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/PipelineState.hpp>

#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Private/RHI/Dx12/Resource/Descriptor.hpp>

#include <ShlObj_core.h>
#include <dxgidebug.h>

#include <glm/gtc/type_ptr.hpp>

#include <Log/Logger.hpp>

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 610;
    __declspec(dllexport) extern const char* D3D12SDKPath   = ".\\D3D12\\";
}

namespace Neon::RHI
{
    static std::unique_ptr<Dx12RenderDevice> s_RenderDevice = nullptr;

    void IRenderDevice::Create(
        Windowing::IWindowApp*     Window,
        const DeviceCreateDesc&    DeviceDesc,
        const SwapchainCreateDesc& SwapchainDesc)
    {
        NEON_ASSERT(!s_RenderDevice);
        s_DescriptorSize = DeviceDesc.Descriptors;
        s_RenderDevice.reset(NEON_NEW Dx12RenderDevice(DeviceDesc));
        s_RenderDevice->PostInitialize(Window, SwapchainDesc);
    }

    IRenderDevice* IRenderDevice::Get()
    {
        return s_RenderDevice.get();
    }

    void IRenderDevice::Destroy()
    {
        NEON_ASSERT(s_RenderDevice);
        s_RenderDevice->Shudown();
        s_RenderDevice = nullptr;

#ifndef NEON_DIST
        Win32::ComPtr<IDXGIDebug1> Debug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&Debug))))
        {
            // DXGI_DEBUG_ALL
            GUID DebugAll = { 0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 };
            Debug->ReportLiveObjects(DebugAll, DXGI_DEBUG_RLO_SUMMARY);
        }
#endif
    }

#if !NEON_DIST
    void RenameObject(IRootSignature* Object, const wchar_t* Name)
    {
        RenameObject(static_cast<Dx12RootSignature*>(Object)->Get(), Name);
    }

    void RenameObject(IPipelineState* Object, const wchar_t* Name)
    {
        RenameObject(static_cast<Dx12PipelineState*>(Object)->Get(), Name);
    }

    void RenameObject(IGpuResource* Object, const wchar_t* Name)
    {
        RenameObject(dynamic_cast<Dx12GpuResource*>(Object)->GetResource(), Name);
    }

    void RenameObject(IDescriptorHeap* Object, const wchar_t* Name)
    {
        RenameObject(dynamic_cast<Dx12DescriptorHeap*>(Object)->Get(), Name);
    }
#endif

    //

    Dx12RenderDevice::Dx12RenderDevice(
        const DeviceCreateDesc& DeviceDesc)
    {
        NEON_INFO_TAG("Graphics", "Creating DirectX 12 Render Device");

        if (DeviceDesc.EnableGPUDebugger)
        {
            LoadPixRuntime();
        }
        EnableDebugLayerIfNeeded(DeviceDesc);
        CreateFactory();
        CreateDevice();
        CheckDeviceFeatures();
        FillInDescriptorSizes();
    }

    Dx12RenderDevice::~Dx12RenderDevice()
    {
        Dx12PipelineStateCache::Flush();
        Dx12RootSignatureCache::Flush();
        NEON_INFO_TAG("Graphics", "Destroying DirectX 12 Render Device");
    }

    RHI::ISwapchain* Dx12RenderDevice::GetSwapchain()
    {
        return m_Swapchain.get();
    }

    //

    void Dx12RenderDevice::PostInitialize(
        Windowing::IWindowApp*     Window,
        const SwapchainCreateDesc& SwapchainDesc)
    {
        m_MemoryAllocator.reset(NEON_NEW GraphicsMemoryAllocator);
        m_Swapchain.reset(NEON_NEW Dx12Swapchain(Window, SwapchainDesc));
        m_Swapchain->PostInitialize(SwapchainDesc);
        CreateDefaultTextures();
    }

    void Dx12RenderDevice::Shudown()
    {
        m_DefaultTextures = {};
        m_Swapchain->Shutdown();
        m_Swapchain       = nullptr;
        m_MemoryAllocator = nullptr;
    }

    Dx12RenderDevice* Dx12RenderDevice::Get()
    {
        return static_cast<Dx12RenderDevice*>(IRenderDevice::Get());
    }

    IDXGIFactory* Dx12RenderDevice::GetDxgiFactory()
    {
        return m_DxgiFactory.Get();
    }

    IDXGIAdapter* Dx12RenderDevice::GetAdapter()
    {
        return m_Adapter.Get();
    }

    ID3D12Device* Dx12RenderDevice::GetDevice()
    {
        return m_Device.Get();
    }

    uint32_t Dx12RenderDevice::GetDescriptorSize(
        D3D12_DESCRIPTOR_HEAP_TYPE Type) const
    {
        switch (Type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return m_HeapDescriptorSize.CBV;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return m_HeapDescriptorSize.SAM;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            return m_HeapDescriptorSize.RTV;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return m_HeapDescriptorSize.DSV;
        default:
            std::unreachable();
        }
    }

    //

    void Dx12RenderDevice::LoadPixRuntime()
    {
#if !NEON_DIST
        if (GetModuleHandleW(STR("WinPixGpuCapturer.dll")))
        {
            return;
        }

        LPWSTR programFilesPath = nullptr;
        SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

        String pixSearchPath = StringUtils::Format(STR("{}\\Microsoft PIX\\*"), programFilesPath);

        WIN32_FIND_DATA findData;
        String          NewestVersionFound;

        HANDLE hFind = INVALID_HANDLE_VALUE;
        try
        {
            hFind = FindFirstFile(pixSearchPath.c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
                        (findData.cFileName[0] != '.'))
                    {
                        String FoundFile = findData.cFileName;
                        if (NewestVersionFound != FoundFile)
                        {
                            NewestVersionFound = std::move(FoundFile);
                        }
                    }
                } while (FindNextFile(hFind, &findData) != 0);
            }
        }
        catch (...)
        {
            NEON_WARNING_TAG("Graphics", "Couldn't find a PIX gpu debugger");
        }

        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
        }

        if (NewestVersionFound.empty())
        {
            NEON_WARNING_TAG("Graphics", "Couldn't find a PIX gpu debugger");
            return;
        }

        pixSearchPath.pop_back();
        pixSearchPath += NewestVersionFound + STR("\\WinPixGpuCapturer.dll");
        if (!LoadLibraryW(pixSearchPath.c_str()))
        {
            NEON_WARNING_TAG("Graphics", "Couldn't find a PIX gpu debugger");
        }
#endif
    }

    void Dx12RenderDevice::EnableDebugLayerIfNeeded(
        const DeviceCreateDesc& DeviceDesc)
    {
#if !NEON_DIST
        Win32::ComPtr<ID3D12Debug1> DebugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)));
        if (DeviceDesc.EnableDebugLayer)
        {
            DebugController->EnableDebugLayer();
        }
        if (DeviceDesc.EnableGpuBasedValidation)
        {
            DebugController->SetEnableGPUBasedValidation(true);
        }
#endif
    }

    void Dx12RenderDevice::CreateFactory()
    {
        ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_DxgiFactory)));
    }

    void Dx12RenderDevice::CreateDevice()
    {
        DXGI_ADAPTER_DESC Desc;
        m_Adapter = GetBestAdapter();
        m_Adapter->GetDesc(&Desc);

        NEON_TRACE_TAG("Graphics", "Vendor: {:X}", Desc.VendorId);
        NEON_TRACE_TAG("Graphics", "Device: {:X}", Desc.DeviceId);
        NEON_TRACE_TAG("Graphics", "Revision: {}", Desc.Revision);
        NEON_TRACE_TAG("Graphics", "Dedicated Video Memory: {} Gb", Desc.DedicatedVideoMemory / (1024.f * 1024.f * 1024.f));
        NEON_TRACE_TAG("Graphics", "Dedicated System Memory: {} Gb", Desc.DedicatedSystemMemory / (1024.f * 1024.f * 1024.f));
        NEON_TRACE_TAG("Graphics", "Shared System Memory: {} Gb", Desc.SharedSystemMemory / (1024.f * 1024.f * 1024.f));
        NEON_TRACE_TAG("Graphics", "Description: {}", StringUtils::Transform<StringU8>(Desc.Description));

        ThrowIfFailed(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));
    }

    const Dx12DeviceFeatures& Dx12RenderDevice::GetFeatures() const
    {
        return m_DeviceFeatures;
    }

    Dx12ShaderCompiler* Dx12RenderDevice::GetShaderCompiler()
    {
        return &m_Compiler;
    }

    GraphicsMemoryAllocator* Dx12RenderDevice::GetAllocator()
    {
        return m_MemoryAllocator.get();
    }

    IResourceStateManager* Dx12RenderDevice::GetStateManager()
    {
        return m_MemoryAllocator->GetStateManager();
    }

    const Ptr<ITexture>& Dx12RenderDevice::GetDefaultTexture(DefaultTextures Type) const
    {
        return m_DefaultTextures[size_t(Type)];
    }

    void Dx12RenderDevice::CheckDeviceFeatures()
    {
        m_DeviceFeatures.Initialize(m_Device.Get());
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

    //

    void Dx12RenderDevice::FillInDescriptorSizes()
    {
        m_HeapDescriptorSize.RTV = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_HeapDescriptorSize.DSV = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        m_HeapDescriptorSize.SAM = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        m_HeapDescriptorSize.CBV = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    void Dx12RenderDevice::CreateDefaultTextures()
    {
        // Debug miplevels
        auto Desc2D   = ResourceDesc::Tex2D(EResourceFormat::R8G8B8A8_UNorm, 1, 1, 1);
        auto Desc3D   = ResourceDesc::Tex3D(EResourceFormat::R8G8B8A8_UNorm, 1, 1, 1);
        auto DescCube = ResourceDesc::TexCube(EResourceFormat::R8G8B8A8_UNorm, 1, 1, 1);

        SubresourceDesc Subresource{
            .RowPitch   = 4,
            .SlicePitch = 4,
        };
        std::span<SubresourceDesc>     Subresource2D{};
        std::span<SubresourceDesc>     Subresource3D{};
        std::array<SubresourceDesc, 6> SubresourceCube{};

        auto CreateSubresources = [&]
        {
            Subresource2D   = { &Subresource, 1 };
            Subresource3D   = { &Subresource, 1 };
            SubresourceCube = { Subresource, Subresource, Subresource,
                                Subresource, Subresource, Subresource };
        };

        Subresource.Data = glm::value_ptr(Colors::Magenta);
        CreateSubresources();

        PendingResource MagentaTexture2D(Desc2D, Subresource2D);
        PendingResource MagentaTexture3D(Desc3D, Subresource3D);
        PendingResource MagentaTextureCube(DescCube, SubresourceCube);

        Subresource.Data = glm::value_ptr(Colors::White);
        CreateSubresources();

        PendingResource WhiteTexture2D(Desc2D, Subresource2D);
        PendingResource WhiteTexture3D(Desc3D, Subresource3D);
        PendingResource WhiteTextureCube(DescCube, SubresourceCube);

        Subresource.Data = glm::value_ptr(Colors::Black);
        CreateSubresources();

        PendingResource BlackTexture2D(Desc2D, Subresource2D);
        PendingResource BlackTexture3D(Desc3D, Subresource3D);
        PendingResource BlackTextureCube(DescCube, SubresourceCube);

        //

        auto CommandQueue = m_Swapchain->GetQueue(CommandQueueType::Graphics);

        auto LoadTexture = [this, CommandQueue](PendingResource& Resource, DefaultTextures Type)
        {
            m_DefaultTextures[size_t(Type)] = Resource.Access<ITexture>(CommandQueue);
        };

        LoadTexture(MagentaTexture2D, DefaultTextures::Magenta_2D);
        LoadTexture(MagentaTexture3D, DefaultTextures::Magenta_3D);
        LoadTexture(MagentaTextureCube, DefaultTextures::Magenta_Cube);

        LoadTexture(WhiteTexture2D, DefaultTextures::White_2D);
        LoadTexture(WhiteTexture3D, DefaultTextures::White_3D);
        LoadTexture(WhiteTextureCube, DefaultTextures::White_Cube);

        LoadTexture(BlackTexture2D, DefaultTextures::Black_2D);
        LoadTexture(BlackTexture3D, DefaultTextures::Black_3D);
        LoadTexture(BlackTextureCube, DefaultTextures::Black_Cube);
    }
} // namespace Neon::RHI