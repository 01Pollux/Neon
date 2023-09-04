#include <ResourcePCH.hpp>
#include <Asset/Asset.hpp>

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
        return m_AssetPath;
    }

    void IAsset::SetPath(
        StringU8 Path) noexcept
    {
        m_AssetPath = std::move(Path);
        MarkDirty();
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