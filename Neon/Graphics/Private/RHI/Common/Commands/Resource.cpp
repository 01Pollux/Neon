#include <GraphicsPCH.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::RHI
{
    IGpuResource::IGpuResource(
        ISwapchain* Swapchain) :
        m_OwningSwapchain(Swapchain)
    {
    }
} // namespace Neon::RHI