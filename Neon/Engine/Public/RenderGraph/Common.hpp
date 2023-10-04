#pragma once

#include <RHI/Resource/Views/GenericView.hpp>
#include <RHI/Resource/Resource.hpp>

#include <Math/Size2.hpp>
#include <Core/String.hpp>
#include <Math/Colors.hpp>

namespace Neon::RG
{
    enum class ResourceReadAccess : uint8_t
    {
        PixelShader,
        NonPixelShader,
        Any
    };

    //

    enum class PassQueueType : uint8_t
    {
        Unknown,
        Direct,
        Compute
    };

    enum class EPassFlags : uint8_t
    {
        Cull,
        // UAVBarrier,

        _Last_Enum,
    };
    using MPassFlags = Bitmask<EPassFlags>;

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

        _Last_Enum,
    };
    using MResourceFlags = Bitmask<EResourceFlags>;

    //

    class ResourceViewId;

    class ResourceId
    {
    public:
        constexpr ResourceId() = default;

        constexpr explicit ResourceId(
            const StringU8& Name) :
            m_Id(StringUtils::Hash(Name))
#ifndef NEON_DIST
            ,
            m_Name(Name)
#endif
        {
        }

        /// <summary>
        /// Get resource id
        /// </summary>
        [[nodiscard]] constexpr size_t Get() const noexcept
        {
            return m_Id;
        }

        /// <summary>
        /// Get resource name (non-dist only)
        /// </summary>
        [[nodiscard]] constexpr const StringU8& GetName() const noexcept
        {
#ifndef NEON_DIST
            return m_Name;
#else
            return StringUtils::Empty<StringU8>;
#endif
        }

        constexpr auto operator<=>(
            const ResourceId& Other) const noexcept
        {
            return m_Id <=> Other.m_Id;
        }

        constexpr operator bool() const noexcept
        {
            return m_Id != std::numeric_limits<size_t>::max();
        }

        /// <summary>
        /// Create a resource view id
        /// </summary>
        [[nodiscard]] constexpr ResourceViewId CreateView(
            const StringU8& ViewName) const;

    private:
        size_t m_Id = std::numeric_limits<size_t>::max();
#ifndef NEON_DIST
        StringU8 m_Name;
#endif
    };

    //

    class ResourceViewId
    {
    public:
        ResourceViewId() = default;

        constexpr explicit ResourceViewId(
            const StringU8& Name,
            const StringU8& ViewName) :
            ResourceViewId(ResourceId(Name), ViewName)
        {
        }

        constexpr explicit ResourceViewId(
            ResourceId      ResId,
            const StringU8& ViewName) :
            m_Resource(ResId),
            m_ViewId(StringUtils::Hash(ViewName))
        {
        }

        /// <summary>
        /// Get resource id
        /// </summary>
        [[nodiscard]] constexpr const ResourceId& GetResource() const noexcept
        {
            return m_Resource;
        }

        /// <summary>
        /// Get resource view id
        /// </summary>
        [[nodiscard]] constexpr size_t Get() const noexcept
        {
            return m_ViewId;
        }

        constexpr operator bool() const noexcept
        {
            return m_ViewId != std::numeric_limits<size_t>::max() && m_Resource;
        }

        auto operator<=>(
            const ResourceViewId& Other) const noexcept
        {
            auto Cmp = m_Resource <=> Other.m_Resource;
            if (Cmp == std::strong_ordering::equal)
            {
                Cmp = m_ViewId <=> Other.m_ViewId;
            }
            return Cmp;
        }

    private:
        ResourceId m_Resource;
        size_t     m_ViewId = std::numeric_limits<size_t>::max();
    };

    //

    inline constexpr ResourceViewId ResourceId::CreateView(
        const StringU8& ViewName) const
    {
        return ResourceViewId(*this, ViewName);
    }

    //

    struct SubresourceView
    {
        uint32_t PlaneIndex = 0;
        uint32_t ArrayIndex = 0;
        uint32_t MipIndex   = std::numeric_limits<uint32_t>::max();

        /// <summary>
        /// Calculate subresource's index
        /// will return -1 if mip index is -1
        /// </summary>
        [[nodiscard]] constexpr uint32_t GetSubresourceIndex(
            uint32_t ArraySize,
            uint32_t MipSize) const;
    };

    //

    class ResourceHandle
    {
    public:
        struct ViewDesc
        {
            RHI::DescriptorViewDesc Desc;
            RHI::Views::Generic     Handle;
            uint32_t                SubresourceIndex;
        };
        using ResourceViewMapType = std::map<ResourceViewId, ViewDesc>;

        ResourceHandle(
            const ResourceId&             Id,
            const Ptr<RHI::IGpuResource>& Texture,
            const RHI::ClearOperationOpt& ClearValue = std::nullopt);

        ResourceHandle(
            const ResourceId&             Id,
            const Ptr<RHI::IGpuResource>& Buffer,
            RHI::GraphicsBufferType       BufferType);

        ResourceHandle(
            const ResourceId&        Id,
            const RHI::ResourceDesc& Desc,
            MResourceFlags           Flags);

        /// <summary>
        /// Get resource id
        /// </summary>
        [[nodiscard]] const ResourceId& GetId() const noexcept;

        /// <summary>
        /// Create a resource view with name
        /// </summary>
        void CreateView(
            const ResourceViewId&          ViewId,
            const RHI::DescriptorViewDesc& Desc,
            const SubresourceView&         Subresource);

        /// <summary>
        /// Get resource views
        /// </summary>
        [[nodiscard]] ResourceViewMapType& GetViews() noexcept;

        /// <summary>
        /// Get resource views
        /// </summary>
        [[nodiscard]] const ResourceViewMapType& GetViews() const noexcept;

        /// <summary>
        /// Get resource desc
        /// </summary>
        [[nodiscard]] RHI::ResourceDesc& GetDesc() noexcept;

        /// <summary>
        /// Get resource desc
        /// </summary>
        [[nodiscard]] const RHI::ResourceDesc& GetDesc() const noexcept;

        /// <summary>
        /// Get resource desc
        /// </summary>
        [[nodiscard]] const MResourceFlags& GetFlags() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IGpuResource>& Get() const noexcept;

        /// <summary>
        /// Set the underlying resource
        /// </summary>
        [[nodiscard]] void Set(
            const Ptr<RHI::IGpuResource>& Resource,
            bool                          Rename = true) noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] RHI::GpuTexture AsTexture() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] RHI::GpuBuffer AsBuffer() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] RHI::GpuUploadBuffer AsUploadBuffer() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] RHI::GpuReadbackBuffer AsReadbackBuffer() const noexcept;

        /// <summary>
        /// check if texture is window sized
        /// </summary>
        [[nodiscard]] bool IsWindowSizedTexture() const noexcept;

        /// <summary>
        /// make texture window sized
        /// </summary>
        [[nodiscard]] void MakeWindowSizedTexture(
            bool State = true) noexcept;

        /// <summary>
        /// check if resource is imported
        /// </summary>
        [[nodiscard]] bool IsImported() const noexcept;

        /// <summary>
        /// Apply name to the resource
        /// </summary>
        void SetName();

    private:
        Ptr<RHI::IGpuResource> m_Resource;
        ResourceViewMapType    m_ViewMap;

        ResourceId        m_Id;
        RHI::ResourceDesc m_Desc;
        MResourceFlags    m_Flags;
    };
} // namespace Neon::RG