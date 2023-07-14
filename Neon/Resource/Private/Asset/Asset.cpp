#include <ResourcePCH.hpp>
#include <Asset/Asset.hpp>

namespace Neon::AAsset
{
    IAsset::IAsset(
        const Handle& NewHandle) :
        m_Handle(NewHandle)
    {
    }

    const Handle& IAsset::GetHandle() const noexcept
    {
        return m_Handle;
    }
} // namespace Neon::AAsset