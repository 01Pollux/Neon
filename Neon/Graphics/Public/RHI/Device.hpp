#pragma once

namespace Neon::RHI
{
	class IRenderDevice
	{
	public:
		static IRenderDevice* CreateGlobal();
		static void DestroyGlobal();

		/// <summary>
		/// Gets the global render device.
		/// </summary>
		[[nodiscard]] static IRenderDevice* Get();

		virtual ~IRenderDevice() = default;
	};
}