#pragma once

#include <RHI/Resource/Common.hpp>
#include <Math/Size2.hpp>
#include <Core/String.hpp>
#include <Math/Colors.hpp>
#include <optional>

namespace Neon::RG
{
    enum class ClearType : uint8_t
    {
        /// <summary>
        /// Don't clear render target view
        /// </summary>
        RTV_Ignore,
        /// <summary>
        /// Clear render target view with the specified color
        /// </summary>
        RTV_Color,

        /// <summary>
        /// Don't clear the depth and stencil view
        /// </summary>
        DSV_Ignore,
        /// <summary>
        /// Clear the depth view only
        /// </summary>
        DSV_Depth,
        /// <summary>
        /// Clear the stencil view only
        /// </summary>
        DSV_Stencil,
        /// <summary>
        /// Clear both depth and stencil view
        /// </summary>
        DSV_DepthStencil,
    };

    //

    enum class ResourceReadAccess : uint8_t
    {
        PixelShader,
        NonPixelShader,
        Any
    };

    struct ConstantBufferViewDesc
    {
        uint32_t ViewOffset;
        uint32_t ViewSize;
    };

    //

    enum class PassQueueType : uint8_t
    {
        Unknown,
        Direct,
        Compute,
        Copy
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

    //

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

    class ResourceViewId
    {
    public:
        ResourceViewId() = default;

        constexpr explicit ResourceViewId(
            const wchar_t* Name,
            const wchar_t* ViewName,
            uint32_t       SubresourceIndex = uint32_t(-1)) :
            ResourceViewId(ResourceId(Name), ViewName, SubresourceIndex)
        {
        }

        constexpr explicit ResourceViewId(
            ResourceId     ResId,
            const wchar_t* ViewName,
            uint32_t       SubresourceIndex = uint32_t(-1)) :
            m_Resource(ResId),
            m_ViewId(StringUtils::Hash(ViewName) + SubresourceIndex),
            m_SubresourceIndex(SubresourceIndex)
        {
        }

        [[nodiscard]] constexpr const ResourceId& GetResource() const noexcept
        {
            return m_Resource;
        }

        [[nodiscard]] constexpr size_t Get() const noexcept
        {
            return m_ViewId;
        }

        [[nodiscard]] constexpr uint32_t GetSubresourceIndex() const noexcept
        {
            return m_SubresourceIndex;
        }

        auto operator<=>(const ResourceViewId&) const noexcept = default;

    private:
        ResourceId m_Resource;
        size_t     m_ViewId           = std::numeric_limits<size_t>::max();
        uint32_t   m_SubresourceIndex = std::numeric_limits<uint32_t>::max();
    };

    //

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