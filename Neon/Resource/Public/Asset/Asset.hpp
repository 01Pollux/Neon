#pragma once

#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    class IAsset
    {
    public:
        IAsset(
            const Handle& AssetGuid,
            StringU8      Path);

        virtual ~IAsset() = default;

        /// <summary>
        /// Gets the asset guid.
        /// </summary>
        [[nodiscard]] const Handle& GetGuid() const noexcept;

        /// <summary>
        /// Get the asset path.
        /// </summary>
        [[nodiscard]] const StringU8& GetPath() const noexcept;

        /// <summary>
        /// Query if the asset is dirty.
        /// </summary>
        [[nodiscard]] bool IsDirty() const noexcept;

        /// <summary>
        /// Marks the asset as dirty.
        /// </summary>
        void MarkDirty(
            bool IsDirty = true) noexcept;

    protected:
        StringU8     m_AssetPath;
        const Handle m_AssetGuid;
        bool         m_IsDirty = true;
    };
} // namespace Neon::AAsset