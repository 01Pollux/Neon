#pragma once

namespace Neon::RHI
{
	enum class RenderDeviceType : unsigned char
	{
		Unknown,
		DirectX12,
	};

	class IRenderDevice
	{
	public:
		template<RenderDeviceType>
		static IRenderDevice* CreateGlobal();
		template<RenderDeviceType>
		static void DestroyGlobal();

		/// <summary>
		/// Gets the global render device.
		/// </summary>
		[[nodiscard]] static IRenderDevice* Get();

		virtual ~IRenderDevice() = default;
	};
}