#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
	void ThrowIfFailed(HRESULT Result)
	{
		NEON_ASSERT(SUCCEEDED(Result), StringUtils::Format("Bad HRESULT returned. (Code: 0x{:X})", uint32_t(Result)));
	}
}