#pragma once

#include <Asset/Handle.hpp>
#include <future>
#include <variant>

namespace Neon::Asset
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
        /// Set the asset path.
        /// </summary>
        [[nodiscard]] void SetPath(
            StringU8 Path) noexcept;

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

    //

    template<typename _Ty = IAsset>
        requires std::is_base_of_v<IAsset, _Ty>
    struct AssetTaskPtr
    {
        AssetTaskPtr() = default;

        AssetTaskPtr(
            std::future<Ptr<IAsset>> Asset) :
            m_Asset(std::move(Asset))
        {
        }

        AssetTaskPtr(
            Ptr<_Ty> Asset) :
            m_Asset(std::move(Asset))
        {
        }

        //

        void operator=(
            std::future<Ptr<IAsset>> Asset) noexcept
        {
            m_Asset = std::move(Asset);
        }

        void operator=(
            Ptr<_Ty> Asset) noexcept
        {
            m_Asset = std::move(Asset);
        }

        //

        [[nodiscard]] operator bool() const noexcept
        {
            return m_Asset.index() == 1 ? std::get<1>(m_Asset) != nullptr : std::get<0>(m_Asset).valid();
        }

        [[nodiscard]] bool operator!() const noexcept
        {
            return !operator bool();
        }

        [[nodiscard]] auto& Get() const
        {
            if (m_Asset.index() != 1) [[unlikely]]
            {
                m_Asset = std::dynamic_pointer_cast<_Ty>(std::get<0>(m_Asset).get());
            }
            return std::get<1>(m_Asset);
        }

        [[nodiscard]] auto& operator->() const
        {
            return Get();
        }

        [[nodiscard]] const Ptr<_Ty>& operator*() const
        {
            return Get();
        }

        [[nodiscard]] operator const Ptr<_Ty>&() const noexcept
        {
            return Get();
        }

    private:
        mutable std::variant<
            std::future<Ptr<IAsset>>,
            Ptr<_Ty>>
            m_Asset;
    };
} // namespace Neon::Asset