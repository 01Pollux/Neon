#pragma once

#include <RHI/Resource/Common.hpp>
#include <Math/Size2.hpp>
#include <Core/String.hpp>
#include <Math/Colors.hpp>
#include <optional>

namespace Neon::RG
{
    enum class PassQueueType : uint8_t
    {
        Unknown,
        Direct,
        Compute,
        Copy
    };

    class ResourceId
    {
    public:
        ResourceId() = default;

        constexpr explicit ResourceId(
            const wchar_t* Name) :
            m_ResourceName(Name),
            m_Id(StringUtils::Hash(Name))
        {
        }

        [[nodiscard]] constexpr size_t Get() const noexcept
        {
            return m_Id;
        }

        [[nodiscard]] constexpr const wchar_t* GetName() const noexcept
        {
#if !NEON_DIDST
            return m_ResourceName;
#else
            return L"";
#endif
        }

        constexpr auto operator<=>(
            const ResourceId& Other) const noexcept
        {
            return m_Id <=> Other.m_Id;
        }

    private:
#if !NEON_DIDST
        const wchar_t* m_ResourceName = nullptr;
#endif
        size_t m_Id = std::numeric_limits<size_t>::max();
    };

    //

    enum class EResourceFlags : uint8_t
    {
        /// <summary>
        /// Resource Was imported
        /// </summary>
        Imported,

        /// <summary>
        /// Texture will have a window width's and height
        /// </summary>
        WindowSizeDependent,
    };
    using MResourceFlags = Bitmask<EResourceFlags>;

    //

    struct ClearOperation
    {
        RHI::EResourceFormat Format;
        union {
            Color4 Color;
            struct
            {
                float   Depth;
                uint8_t Stencil;
            } DepthStencil;
        };
    };

    using ClearOperationOpt = std::optional<ClearOperation>;

    struct ResourceDesc : RHI::ResourceDesc
    {
    public:
        /// <summary>
        /// Set clear value for render target view
        /// </summary>
        void SetClearValue(
            RHI::EResourceFormat Format,
            const Color4&        Color);

        /// <summary>
        /// Set clear value for depth stencil view
        /// </summary>
        void SetClearValue(
            RHI::EResourceFormat Format,
            float                Depth,
            uint8_t              Stencil);

        /// <summary>
        /// Set clear value for render target view
        /// </summary>
        void SetClearValue(
            const Color4& Color);

        /// <summary>
        /// Set clear value for depth stencil view
        /// </summary>
        void SetClearValue(
            float   Depth,
            uint8_t Stencil);

        /// <summary>
        /// Set clear value for depth stencil view
        /// </summary>
        void SetClearValue(
            const ClearOperationOpt& Op);

        /// <summary>
        /// Unset clear value
        /// </summary>
        void UnsetClearValue();

    public:
        ClearOperationOpt ClearValue = std::nullopt;
    };
} // namespace Neon::RG