#pragma once

#include <Math/Vector.hpp>
#include <RHI/Resource/Common.hpp>
#include <RHI/Swapchain.hpp>
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

    //

    /// <summary>
    /// Upload resource asynchronously and wait for it to be ready when accessed.
    /// </summary>
    template<typename _ResourceTy, bool _Owning>
    class SyncGpuResourceT
    {
    public:
        using ResourcePtr = std::conditional_t<_Owning, UPtr<_ResourceTy>, Ptr<_ResourceTy>>;

        SyncGpuResourceT() = default;

        SyncGpuResourceT(
            ResourcePtr Resource,
            uint64_t    CopyId) :
            m_Resource(std::move(Resource)),
            m_CopyId(CopyId)
        {
        }

        SyncGpuResourceT(
            Ptr<IGpuResource> Resource) :
            m_Resource(std::move(Resource))
        {
        }

    public:
        /// <summary>
        /// Returns the resource after it has been copied.
        /// </summary>
        [[nodiscard]] const ResourcePtr& operator->() const
        {
            return Get();
        }

        /// <summary>
        /// Returns the resource after it has been copied.
        /// </summary>
        [[nodiscard]] ResourcePtr& operator->()
        {
            return Get();
        }

        /// <summary>
        /// Returns the resource after it has been copied.
        /// </summary>
        [[nodiscard]] operator const ResourcePtr&() const
        {
            return Get();
        }

        /// <summary>
        /// Returns the resource after it has been copied.
        /// </summary>
        [[nodiscard]] const ResourcePtr& Get() const
        {
            WaitForCopy();
            return m_Resource;
        }

        /// <summary>
        /// Returns the resource after it has been copied.
        /// </summary>
        [[nodiscard]] ResourcePtr Release()
        {
            WaitForCopy();
            return std::move(m_Resource);
        }

    protected:
        /// <summary>
        /// Wait for copy operation to complete.
        /// </summary>
        void WaitForCopy() const
        {
            if (m_CopyId && m_Resource) [[unlikely]]
            {
                auto DirectQueue  = ISwapchain::Get()->GetQueue(RHI::CommandQueueType::Graphics);
                auto ComputeQueue = ISwapchain::Get()->GetQueue(RHI::CommandQueueType::Compute);

                ISwapchain::Get()->WaitForCopy(DirectQueue, m_CopyId);
                ISwapchain::Get()->WaitForCopy(ComputeQueue, m_CopyId);

                m_CopyId = 0;
            }
        }

    protected:
        ResourcePtr      m_Resource;
        mutable uint64_t m_CopyId = 0;
    };

    template<bool _Owning>
    class SyncBufferT : public SyncGpuResourceT<IBuffer, _Owning>
    {
    public:
        using SyncGpuResourceT<IBuffer, _Owning>::SyncGpuResourceT;

        SyncBufferT(
            const BufferDesc& Buffer)
        {
            this->m_Resource.reset(IBuffer::Create(
                Buffer));
        }

        SyncBufferT(
            const ResourceDesc&    Desc,
            const SubresourceDesc& Subresources)
        {
            this->m_Resource.reset(IBuffer::Create(
                Desc,
                Subresources,
                this->m_CopyId));
        }
    };

    template<bool _Owning>
    class SyncTextureT : public SyncGpuResourceT<ITexture, _Owning>
    {
    public:
        using SyncGpuResourceT<ITexture, _Owning>::SyncGpuResourceT;

        SyncTextureT(
            const TextureRawImage& ImageData)
        {
            this->m_Resource.reset(ITexture::Create(
                ImageData,
                this->m_CopyId));
        }

        SyncTextureT(
            const ResourceDesc& Desc)
        {
            this->m_Resource.reset(ITexture::Create(
                Desc));
        }

        SyncTextureT(
            const ResourceDesc&              Desc,
            std::span<const SubresourceDesc> Subresources)
        {
            this->m_Resource.reset(ITexture::Create(
                Desc,
                Subresources,
                this->m_CopyId));
        }
    };

    //

    using SSyncBuffer = SyncBufferT<false>;
    using USyncBuffer = SyncBufferT<true>;

    using SSyncTexture = SyncTextureT<false>;
    using USyncTexture = SyncTextureT<true>;
} // namespace Neon::RHI