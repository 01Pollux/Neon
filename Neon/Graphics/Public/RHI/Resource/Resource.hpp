#pragma once

#include <Math/Vector.hpp>
#include <RHI/Resource/Common.hpp>
#include <RHI/Commands/Common.hpp>
#include <variant>
#include <span>

namespace Neon::RHI
{
    class ISwapchain;
    class ICommandQueue;
    struct DescriptorHeapHandle;

    //

    struct ResourceFootprint
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t Depth;
        uint32_t RowPitch;

        EResourceFormat Format;
    };

    struct SubresourceFootprint : public ResourceFootprint
    {
        size_t Offset;
    };

    struct SubresourceDesc
    {
        const void* Data;
        size_t      RowPitch;
        size_t      SlicePitch;
    };

    struct TextureCopyLocation
    {
        IGpuResource*                                Resource;
        std::variant<uint32_t, SubresourceFootprint> Subresource;
    };

    //

    class IGpuResource
    {
    public:
        virtual ~IGpuResource() = default;

        /// <summary>
        /// Query the footprint of the resource.
        /// </summary>
        virtual void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               LinearSize) const = 0;

        /// <summary>
        /// Copy the subresources to the resource
        /// </summary>
        virtual void CopyFrom(
            uint32_t                         FirstSubresource,
            std::span<const SubresourceDesc> Subresources,
            uint64_t&                        CopyId) = 0;

        /// <summary>
        /// Copy the subresource to the resource
        /// </summary>
        void CopyFrom(
            uint32_t               FirstSubresource,
            const SubresourceDesc& Subresource,
            uint64_t&              CopyId)
        {
            std::array Subresources = { Subresource };
            CopyFrom(FirstSubresource, Subresources, CopyId);
        }

        /// <summary>
        /// Get desc of the resource.
        /// </summary>
        [[nodiscard]] const ResourceDesc& GetDesc() const
        {
            return m_Desc;
        }

    protected:
        ResourceDesc m_Desc;
    };

    //

    struct BufferDesc
    {
        size_t         Size;
        MResourceFlags Flags;
    };

    class IBuffer : public virtual IGpuResource
    {
    public:
        using IGpuResource::IGpuResource;

        /// <summary>
        /// Creates a buffer.
        /// </summary>
        [[nodiscard]] static IBuffer* Create(
            const BufferDesc& Desc);

        /// <summary>
        /// Creates a readback buffer.
        /// </summary>
        [[nodiscard]] static IBuffer* Create(
            const BufferDesc&      Desc,
            const SubresourceDesc& Subresource,
            uint64_t&              CopyId);

        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] virtual size_t GetSize() const = 0;

        /// <summary>
        /// Get resource handle in gpu.
        /// </summary>
        [[nodiscard]] virtual GpuResourceHandle GetHandle(
            size_t Offset = 0) const = 0;
    };

    //

    class IUploadBuffer : public virtual IBuffer
    {
    public:
        using IBuffer::IBuffer;

        /// <summary>
        /// Creates an upload buffer.
        /// </summary>
        [[nodiscard]] static IUploadBuffer* Create(
            const BufferDesc& Desc);

        /// <summary>
        /// Creates a readback buffer.
        /// </summary>
        [[nodiscard]] static IUploadBuffer* Create(
            const BufferDesc&      Desc,
            const SubresourceDesc& Subresource,
            uint64_t&              CopyId);

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        [[nodiscard]] virtual uint8_t* Map() = 0;

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        virtual void Unmap() = 0;

        /// <summary>
        /// Maps the buffer to the specified type.
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] _Ty* Map(
            size_t Offset = 0)
        {
            return std::bit_cast<_Ty*>(Map() + Offset);
        }

        /// <summary>
        /// Map and write data to the buffer, then unmap.
        /// </summary>
        void Write(
            size_t      Offset,
            const void* Data,
            size_t      Size)
        {
            auto Mapped = Map<uint8_t>(Offset);
            std::copy_n(static_cast<const uint8_t*>(Data), Size, Mapped);
            Unmap();
        }
    };

    //

    class IReadbackBuffer : public virtual IBuffer
    {
    public:
        using IBuffer::IBuffer;

        /// <summary>
        /// Creates a readback buffer.
        /// </summary>
        [[nodiscard]] static IReadbackBuffer* Create(
            const BufferDesc& Desc);

        /// <summary>
        /// Makes the buffer available for reading by the CPU.
        /// </summary>
        [[nodiscard]] virtual uint8_t* Map() = 0;

        /// <summary>
        /// Makes the buffer available for reading by the CPU.
        /// </summary>
        virtual void Unmap() = 0;
    };

    //

    struct TextureRawImage
    {
        enum class Format : uint8_t
        {
            None,
            Dds,
            Ico,
            Bmp,
            Png,
            Jpeg,
            Jxr,
            Tiff,
            // TODO: Gif
        };
        uint8_t* Data = nullptr;
        size_t   Size = 0;
        Format   Type = Format::None;
    };

    enum class DefaultTextures : uint8_t
    {
        Magenta_2D,
        Magenta_3D,
        Magenta_Cube,

        White_2D,
        White_3D,
        White_Cube,

        Black_2D,
        Black_3D,
        Black_Cube,

        Count,

        Error_2D   = Magenta_2D,
        Error_3D   = Magenta_3D,
        Error_Cube = Magenta_Cube,
    };

    class ITexture : public virtual IGpuResource
    {
    public:
        using IGpuResource::IGpuResource;

        /// <summary>
        /// Creates a texture.
        /// </summary>
        [[nodiscard]] static ITexture* Create(
            const ResourceDesc& Desc);

        /// <summary>
        /// Creates a texture.
        /// </summary>
        [[nodiscard]] static ITexture* Create(
            const ResourceDesc&              Desc,
            std::span<const SubresourceDesc> Subresources,
            uint64_t&                        CopyId);

        /// <summary>
        /// Creates a texture.
        /// </summary>
        [[nodiscard]] static ITexture* Create(
            const TextureRawImage& ImageData,
            uint64_t&              CopyId);

        /// <summary>
        /// Returns the dimensions of the texture.
        /// </summary>
        [[nodiscard]] virtual Vector3I GetDimensions() const = 0;

        /// <summary>
        /// Returns the number of mip levels in the texture.
        /// </summary>
        [[nodiscard]] virtual uint16_t GetMipLevels() const = 0;

        /// <summary>
        /// Get number of subresources in texture
        /// </summary>
        [[nodiscard]] virtual uint32_t GetSubResourceCount() const = 0;

        /// <summary>
        /// Get subresource index from plane, array and mip index
        /// </summary>
        [[nodiscard]] virtual uint32_t GetSubresourceIndex(
            uint32_t PlaneIndex,
            uint32_t ArrayIndex,
            uint32_t MipIndex) const = 0;

    public:
        /// <summary>
        /// Get default texture for specified type.
        /// </summary>
        static [[nodiscard]] const Ptr<ITexture>& GetDefault(
            DefaultTextures Type);
    };

    /// TODO: Rework and remove PendingResource!!!
    class PendingResource
    {
    public:
        PendingResource(
            const ResourceDesc&              Desc,
            std::span<const SubresourceDesc> Subresources)
        {
            uint64_t CopyId = 0;
            m_Resource      = UPtr<ITexture>(ITexture::Create(
                Desc,
                Subresources,
                CopyId));
            m_CopyId        = CopyId;
        }

        PendingResource(
            const TextureRawImage& ImageData)
        {
            uint64_t CopyId = 0;
            m_Resource      = UPtr<ITexture>(ITexture::Create(
                ImageData,
                CopyId));
            m_CopyId        = CopyId;
        }

        PendingResource(
            Ptr<IGpuResource>       Texture,
            std::optional<uint64_t> CopyId) :
            m_Resource(std::move(Texture)),
            m_CopyId(CopyId)
        {
        }

        /// <summary>
        /// Returns the texture once it has been created.
        /// </summary>
        Ptr<IGpuResource> Access(
            ICommandQueue* Queue) const;

        /// <summary>
        /// Returns the texture once it has been created.
        /// </summary>
        Ptr<IGpuResource> Access(
            RHI::CommandQueueType QueueType) const;

        /// <summary>
        /// Returns the texture once it has been created.
        /// </summary>
        template<typename _Ty>
            requires std::derived_from<_Ty, IGpuResource>
        auto Access(
            ICommandQueue* Queue) const
        {
            return std::dynamic_pointer_cast<_Ty>(Access(Queue));
        }

        /// <summary>
        /// Returns the texture once it has been created.
        /// </summary>
        template<typename _Ty>
            requires std::derived_from<_Ty, IGpuResource>
        auto Access(
            RHI::CommandQueueType QueueType) const
        {
            return std::dynamic_pointer_cast<_Ty>(Access(QueueType));
        }

    private:
        Ptr<IGpuResource> m_Resource;

        mutable std::optional<uint64_t> m_CopyId;
    };

    ////

    ///// <summary>
    ///// Upload resource asynchronously and wait for it to be ready when accessed.
    ///// </summary>
    // template<typename _ResourceTy, bool _Owning>
    // class GpuResourceUT
    //{
    // public:
    //     using ResourcePtr = std::conditional_t<_Owning, UPtr<GpuResourceUT>, SPtr<GpuResourceUT>>;

    //    GpuResourceUT() = default;

    //    GpuResourceUT(
    //        const TextureRawImage& ImageData)
    //    {
    //    }

    //    GpuResourceUT(
    //        const ResourceDesc& Desc);

    //    GpuResourceUT(
    //        Ptr<IGpuResource> Resource,
    //        uint64_t          CopyId);

    //    GpuResourceUT(
    //        Ptr<IGpuResource> Resource);

    // protected:
    //     ResourcePtr m_Resource;
    //     uint64_t    m_CopyId = 0;
    // };

    // template<bool _Owning>
    // class BufferUT : public GpuResourceUT<IBuffer, _Owning>
    //{
    // public:
    //     BufferUT(
    //         const BufferDesc& Buffer)
    //     {
    //         m_Resource = ResourcePtr(IBuffer::Create(
    //             Buffer));
    //     }

    //    BufferUT(
    //        const ResourceDesc&    Desc,
    //        const SubresourceDesc& Subresources)
    //    {
    //    }
    //};
} // namespace Neon::RHI