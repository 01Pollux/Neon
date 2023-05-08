#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
	template<>
	IRenderDevice* IRenderDevice::Create<RenderDeviceType::DirectX12>()
	{
		return new Dx12RenderDevice();
	}

	Dx12RenderDevice::Dx12RenderDevice()
	{
		CreateFactory();
		CreateDevice();
	}

	void Dx12RenderDevice::CreateFactory()
	{
		ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&m_Factory)));
	}

	void Dx12RenderDevice::CreateDevice()
	{
		DXGI_ADAPTER_DESC Desc;
		auto Adapter = GetBestAdapter();
		Adapter->GetDesc(&Desc);

		NEON_TRACE_TAG("Graphics", "Vendor: {}", Desc.VendorId);
		NEON_TRACE_TAG("Graphics", "Device: {}", Desc.DeviceId);
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
		size_t BestVideoMemory = 0;
		for (UINT i = 0; m_Factory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			if (FAILED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
			{
				continue;
			}

			DXGI_ADAPTER_DESC Desc;
			Adapter->GetDesc(&Desc);
			if (Desc.DedicatedVideoMemory > BestVideoMemory)
			{
				BestAdapter = Adapter;
				BestVideoMemory = Desc.DedicatedVideoMemory;
			}
		}
		return BestAdapter;
	}
} // Neon::RHI