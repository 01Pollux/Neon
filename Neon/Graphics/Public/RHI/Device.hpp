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
		[[nodiscard]] static IRenderDevice* Create();

		virtual ~IRenderDevice() = default;
	};
}