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
        Compute
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

        _Last_Enum,
    };
    using MResourceFlags = Bitmask<EResourceFlags>;

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

    class ResourceHandle
    {
    public:
        using ViewDesc            = std::pair<RHI::Views::Generic, RHI::DescriptorViewDesc>;
        using ResourceViewMapType = std::map<size_t, ViewDesc>;

        ResourceHandle(
            const ResourceId&             Id,
            const Ptr<RHI::ITexture>&     Texture,
            const RHI::ClearOperationOpt& ClearValue = std::nullopt);

        ResourceHandle(
            const ResourceId&        Id,
            const Ptr<RHI::IBuffer>& Buffer,
            RHI::GraphicsBufferType  BufferType);

        ResourceHandle(
            const ResourceId&        Id,
            const RHI::ResourceDesc& Desc,
            MResourceFlags           Flags,
            RHI::GraphicsBufferType  BufferType = RHI::GraphicsBufferType::Count);

        /// <summary>
        /// Get resource id
        /// </summary>
        [[nodiscard]] const ResourceId& GetId() const noexcept;

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
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IGpuResource>& Get() const noexcept;

        /// <summary>
        /// Set the underlying resource
        /// </summary>
        [[nodiscard]] void Set(
            const Ptr<RHI::IGpuResource>& Resource) noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] Ptr<RHI::ITexture> AsTexture() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] Ptr<RHI::IBuffer> AsBuffer() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] Ptr<RHI::IUploadBuffer> AsUploadBuffer() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] Ptr<RHI::IReadbackBuffer> AsReadbackBuffer() const noexcept;

        /// <summary>
        /// Get buffer type if the resource is a buffer
        /// </summary>
        [[nodiscard]] RHI::GraphicsBufferType GetBufferType() const noexcept;

        /// <summary>
        /// check if texture is window sized
        /// </summary>
        [[nodiscard]] bool IsWindowSizedTexture() const noexcept;

        /// <summary>
        /// check if resource is imported
        /// </summary>
        [[nodiscard]] bool IsImported() const noexcept;

    private:
        Ptr<RHI::IGpuResource> m_Resource;
        ResourceViewMapType    m_ViewMap;

        ResourceId        m_Id;
        RHI::ResourceDesc m_Desc;
        MResourceFlags    m_Flags;

        RHI::GraphicsBufferType m_BufferType : 2;
    };
} // namespace Neon::RG