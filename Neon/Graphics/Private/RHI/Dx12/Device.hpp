#pragma once

#include <RHI/Device.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
	class Dx12RenderDevice : public IRenderDevice
	{
	public:
		Dx12RenderDevice();

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

	private:
		Win32::ComPtr<IDXGIFactory> m_Factory;
		Win32::ComPtr<ID3D12Device> m_Device;
	};
}