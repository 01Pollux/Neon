#include <ResourcePCH.hpp>
#include <Asset/Asset.hpp>

#include <Log/Logger.hpp>

namespace Neon::Asset
{
    IAsset::IAsset(
        const Handle& AssetGuid,
        StringU8      Path) :
        m_AssetGuid(AssetGuid),
        m_AssetPath(std::move(Path))
    {
    }

    const Handle& IAsset::GetGuid() const noexcept
    {
        return m_AssetGuid;
    }

    const StringU8& IAsset::GetPath() const noexcept
    {
        NEON_VALIDATE(!(m_AssetPath.empty() || m_AssetPath.starts_with("..")), "Path '{}' cannot be empty or start with '..'", m_AssetPath);
        return m_AssetPath;
    }

    void IAsset::MarkDirty(
        bool IsDirty) noexcept
    {
        m_IsDirty = IsDirty;
    }

    bool IAsset::IsDirty() const noexcept
    {
        return m_IsDirty;
    }
} // namespace Neon::Asset