#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <Math/Colors.hpp>
#include <RHI/Resource/View.hpp>

#define RG_RESOURCEID(Name)              Renderer::RG::ResourceId(STR(Name))
#define RG_RESOURCEVIEW(Name, View, ...) Renderer::RG::ResourceViewId(STR(Name), STR(View), __VA_ARGS__)

#define RG_RESOURCEID_NAME(Name)        STR(Name), RG_RESOURCEID(Name)
#define RG_RESOURCEVIEW_NAME(Name, ...) STR(Name), RG_RESOURCEVIEW(Name, __VA_ARGS__)

namespace Neon::Rendering
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

    struct ShaderResourceViewDesc
    {
        std::optional<RHI::SRVDesc> HeapDesc;
    };

    struct UnorderedAccessViewDesc
    {
        std::optional<RHI::UAVDesc> HeapDesc;
    };

    struct RenderTargetViewDesc
    {
        std::optional<RHI::RTVDesc> HeapDesc;

        ClearType             ClearType = ClearType::RTV_Ignore;
        std::optional<Color4> ForceColor;
    };

    struct DepthStencilViewDesc
    {
        std::optional<RHI::DSVDesc> HeapDesc;

        ClearType              ClearType = ClearType::DSV_Ignore;
        std::optional<float>   ForceDepth;
        std::optional<uint8_t> ForceStencil;
    };

    enum class ResourceViewDescType : uint8_t
    {
        ConstantBuffer,
        ShaderResource,
        UnorderedAccess,
        RenderTarget,
        DepthStencil
    };

    using ResourceViewDesc = std::variant<
        ConstantBufferViewDesc,
        ShaderResourceViewDesc,
        UnorderedAccessViewDesc,
        RenderTargetViewDesc,
        DepthStencilViewDesc>;

    using ResourceViewDescriptorDesc = std::pair<
        RHI::IDescriptorHeapAllocator::Handle,
        ResourceViewDesc>;

    //

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

    enum class ResourceFlags : uint8_t
    {
        None,

        /// <summary>
        /// Texture will have a window width's and height
        /// </summary>
        WindowSizedTexture = 1 << 0,

        /// <summary>
        /// Resource Was imported
        /// </summary>
        Internal_Imported = 1 << 1,

        /// <summary>
        /// Resource is being used as srv/uav
        /// </summary>
        Internal_UsedInShader = 1 << 2,
    };
    NEON_BITMASK_OPS(ResourceFlags);

    struct ClearOperation
    {
        DXGI_FORMAT Format;
        union {
            Color4                    Color;
            D3D12_DEPTH_STENCIL_VALUE DepthStencil;
        };
    };

    using ClearOperationOpt = std::optional<ClearOperation>;

    struct ResourceDesc : public CD3DX12_RESOURCE_DESC
    {
        using CD3DX12_RESOURCE_DESC::CD3DX12_RESOURCE_DESC;

    public:
        ResourceDesc(
            CD3DX12_RESOURCE_DESC&& Other);

        /// <summary>
        /// Set clear value for render target view
        /// </summary>
        void SetClearValue(
            DXGI_FORMAT   Format,
            const Color4& Color);

        /// <summary>
        /// Set clear value for depth stencil view
        /// </summary>
        void SetClearValue(
            DXGI_FORMAT Format,
            float       Depth,
            uint8_t     Stencil);

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

    //

    class ResourceHandle
    {
        friend class RenderGraphPassResBuilder;

    public:
        using ResourceViewMapType = std::map<size_t, ResourceViewDescriptorDesc>;

        ResourceHandle(
            const ResourceId&                 Id,
            const RHI::GraphicsResource::Ptr& Resource,
            const ClearOperationOpt&          ClearValue = std::nullopt);

        ResourceHandle(
            const ResourceId&                 Id,
            const RHI::GraphicsResource::Ptr& Resource,
            RHI::GraphicsBufferType           BufferType);

        ResourceHandle(
            const ResourceId&       Id,
            const ResourceDesc&     Desc,
            ResourceFlags           Flags,
            RHI::GraphicsBufferType BufferType = RHI::GraphicsBufferType::Count);

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
        [[nodiscard]] ResourceDesc& GetDesc() noexcept;

        /// <summary>
        /// Get resource desc
        /// </summary>
        [[nodiscard]] const ResourceDesc& GetDesc() const noexcept;

        /// <summary>
        /// Get the underlying resource
        /// </summary>
        [[nodiscard]] const RHI::GraphicsResource::Ptr& Get() const noexcept;

        /// <summary>
        /// Set the underlying resource
        /// </summary>
        [[nodiscard]] void Set(
            const RHI::GraphicsResource::Ptr& Resource) noexcept;

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

        /// <summary>
        /// check if resource is shader visible
        /// </summary>
        [[nodiscard]] bool IsShaderVIsible() const noexcept;

    private:
        RHI::GraphicsResource::Ptr m_Resource;
        ResourceViewMapType        m_ViewMap;
        ResourceId                 m_Id;
        ResourceDesc               m_Desc;
        ResourceFlags              m_Flags;
        RHI::GraphicsBufferType    m_BufferType : 2 = RHI::GraphicsBufferType::Count;
    };
} // namespace Neon::Rendering
