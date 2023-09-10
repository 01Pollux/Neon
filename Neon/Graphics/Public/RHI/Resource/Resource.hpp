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

    namespace CSubresourceFlags
    {
        // BC formats with malformed mipchain blocks smaller than 4x4
        static uint32_t Bad_Dxtn_Tails = 1 << 0;

        // Override with a legacy 24 bits-per-pixel format size
        static uint32_t Override_With_24BPP = 1 << 1;

        // Override with a legacy 16 bits-per-pixel format size
        static uint32_t Override_With_16BPP = 1 << 2;

        // Override with a legacy 8 bits-per-pixel format size
        static uint32_t Override_With_8BPP = 1 << 3;

        // Assume pitch is DWORD aligned instead of BYTE aligned
        static uint32_t Assume_Dword_Aligned = 1 << 4;

        // Assume pitch is 16-byte aligned instead of BYTE aligned
        static uint32_t Assume_Word_Aligned = 1 << 4;

        // Assume pitch is 32-byte aligned instead of BYTE aligned
        static uint32_t Assume_32_Bytes_Aligned = 1 << 4;

        // Assume pitch is 64-byte aligned instead of BYTE aligned
        static uint32_t Assume_64_Bytes_Aligned = 1 << 4;

        // Assume pitch is 4096-byte aligned instead of BYTE aligned
        static uint32_t Assume_4K_Bytes_Aligned = 1 << 4;
    } // namespace CSubresourceFlags

    /// <summary>
    /// Computes the image row pitch in bytes, and the slice ptich (size in bytes of the image)
    /// based on DXGI format, width, and height
    /// https://github.com/microsoft/DirectXTex/blob/main/DirectXTex/DirectXTexUtil.cpp#L918
    /// </summary>
    [[nodiscard]] static SubresourceDesc ComputeSubresource(
        EResourceFormat Format,
        const void*     Data,
        uint32_t        Width,
        uint32_t        Height,
        uint32_t        Flags = 0)
    {
        SubresourceDesc Desc{ .Data = Data };
        switch (Format)
        {
        case EResourceFormat::BC1_Typeless:
        case EResourceFormat::BC1_UNorm:
        case EResourceFormat::BC1_UNorm_SRGB:
        case EResourceFormat::BC4_Typeless:
        case EResourceFormat::BC4_UNorm:
        case EResourceFormat::BC4_SNorm:
            if (Flags & CSubresourceFlags::Bad_Dxtn_Tails)
            {
                const size_t nbw = Width >> 2;
                const size_t nbh = Height >> 2;
                Desc.RowPitch    = std::max<uint64_t>(1u, uint64_t(nbw) * 8u);
                Desc.SlicePitch  = std::max<uint64_t>(1u, Desc.RowPitch * uint64_t(nbh));
            }
            else
            {
                const uint64_t nbw = std::max<uint64_t>(1u, (uint64_t(Width) + 3u) / 4u);
                const uint64_t nbh = std::max<uint64_t>(1u, (uint64_t(Height) + 3u) / 4u);
                Desc.RowPitch      = nbw * 8u;
                Desc.SlicePitch    = Desc.RowPitch * nbh;
            }
            break;

        case EResourceFormat::BC2_Typeless:
        case EResourceFormat::BC2_UNorm:
        case EResourceFormat::BC2_UNorm_SRGB:
        case EResourceFormat::BC3_Typeless:
        case EResourceFormat::BC3_UNorm:
        case EResourceFormat::BC3_UNorm_SRGB:
        case EResourceFormat::BC5_Typeless:
        case EResourceFormat::BC5_UNorm:
        case EResourceFormat::BC5_SNorm:
        case EResourceFormat::BC6H_Typeless:
        case EResourceFormat::BC6H_UF16:
        case EResourceFormat::BC6H_SF16:
        case EResourceFormat::BC7_Typeless:
        case EResourceFormat::BC7_UNorm:
        case EResourceFormat::BC7_UNorm_SRGB:
            if (Flags & CSubresourceFlags::Bad_Dxtn_Tails)
            {
                const size_t nbw = Width >> 2;
                const size_t nbh = Height >> 2;
                Desc.RowPitch    = std::max<uint64_t>(1u, uint64_t(nbw) * 16u);
                Desc.SlicePitch  = std::max<uint64_t>(1u, Desc.RowPitch * uint64_t(nbh));
            }
            else
            {
                const uint64_t nbw = std::max<uint64_t>(1u, (uint64_t(Width) + 3u) / 4u);
                const uint64_t nbh = std::max<uint64_t>(1u, (uint64_t(Height) + 3u) / 4u);
                Desc.RowPitch      = nbw * 16u;
                Desc.SlicePitch    = Desc.RowPitch * nbh;
            }
            break;

        case EResourceFormat::R8G8_B8G8_UNorm:
        case EResourceFormat::G8R8_G8B8_UNorm:
        case EResourceFormat::YUY2:
            Desc.RowPitch   = ((uint64_t(Width) + 1u) >> 1) * 4u;
            Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
            break;

        case EResourceFormat::Y210:
        case EResourceFormat::Y216:
            Desc.RowPitch   = ((uint64_t(Width) + 1u) >> 1) * 8u;
            Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
            break;

        case EResourceFormat::NV12:
        case EResourceFormat::Opaque_420:
            if ((Height % 2) != 0) [[unlikely]]
            {
                Desc.Data = nullptr;
                break;
            }
            Desc.RowPitch   = ((uint64_t(Width) + 1u) >> 1) * 2u;
            Desc.SlicePitch = Desc.RowPitch * (uint64_t(Height) + ((uint64_t(Height) + 1u) >> 1));
            break;

        case EResourceFormat::P010:
        case EResourceFormat::P016:
            if ((Height % 2) != 0) [[unlikely]]
            {
                Desc.Data = nullptr;
                break;
            }
            [[fallthrough]];

        case EResourceFormat::NV11:
            Desc.RowPitch   = ((uint64_t(Width) + 3u) >> 2) * 4u;
            Desc.SlicePitch = Desc.RowPitch * uint64_t(Height) * 2u;
            break;

        case EResourceFormat::P208:
            Desc.RowPitch   = ((uint64_t(Width) + 1u) >> 1) * 2u;
            Desc.SlicePitch = Desc.RowPitch * uint64_t(Height) * 2u;
            break;

        case EResourceFormat::V208:
            if ((Height % 2) != 0) [[unlikely]]
            {
                Desc.Data = nullptr;
                break;
            }
            Desc.RowPitch   = uint64_t(Width);
            Desc.SlicePitch = Desc.RowPitch * (uint64_t(Height) + (((uint64_t(Height) + 1u) >> 1) * 2u));
            break;

        case EResourceFormat::V408:
            Desc.RowPitch   = uint64_t(Width);
            Desc.SlicePitch = Desc.RowPitch * (uint64_t(Height) + (uint64_t(Height >> 1) * 4u));
            break;

        default:
        {
            size_t BPP;

            if (Flags & CSubresourceFlags::Override_With_24BPP)
                BPP = 24;
            else if (Flags & CSubresourceFlags::Override_With_16BPP)
                BPP = 16;
            else if (Flags & CSubresourceFlags::Override_With_8BPP)
                BPP = 8;
            else
                BPP = BitsPerPixel(Format);

            if (!BPP)
            {
                Desc.Data = nullptr;
                break;
            }

            if (Flags & (CSubresourceFlags::Assume_Dword_Aligned |
                         CSubresourceFlags::Assume_Word_Aligned |
                         CSubresourceFlags::Assume_32_Bytes_Aligned |
                         CSubresourceFlags::Assume_64_Bytes_Aligned |
                         CSubresourceFlags::Assume_4K_Bytes_Aligned))
            {
                if (Flags & CSubresourceFlags::Assume_4K_Bytes_Aligned)
                {
                    Desc.RowPitch   = ((uint64_t(Width) * BPP + 32767u) / 32768u) * 4096u;
                    Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
                }
                else if (Flags & CSubresourceFlags::Assume_64_Bytes_Aligned)
                {
                    Desc.RowPitch   = ((uint64_t(Width) * BPP + 511u) / 512u) * 64u;
                    Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
                }
                else if (Flags & CSubresourceFlags::Assume_32_Bytes_Aligned)
                {
                    Desc.RowPitch   = ((uint64_t(Width) * BPP + 255u) / 256u) * 32u;
                    Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
                }
                else if (Flags & CSubresourceFlags::Assume_Word_Aligned)
                {
                    Desc.RowPitch   = ((uint64_t(Width) * BPP + 127u) / 128u) * 16u;
                    Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
                }
                else // DWORD alignment
                {
                    // Special computation for some incorrectly created DDS files based on
                    // legacy DirectDraw assumptions about Desc.RowPitch alignment
                    Desc.RowPitch   = ((uint64_t(Width) * BPP + 31u) / 32u) * sizeof(uint32_t);
                    Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
                }
            }
            else
            {
                // Default byte alignment
                Desc.RowPitch   = (uint64_t(Width) * BPP + 7u) / 8u;
                Desc.SlicePitch = Desc.RowPitch * uint64_t(Height);
            }
            break;
        }
        }
        return Desc;
    }

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
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            uint64_t&                          CopyId,
            std::optional<RHI::MResourceState> InitialState = std::nullopt) = 0;

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
        static constexpr auto DefaultResourcestate = RHI::MResourceState_Common;

        using IGpuResource::IGpuResource;

        /// <summary>
        /// Creates a buffer.
        /// </summary>
        [[nodiscard]] static IBuffer* Create(
            const BufferDesc&          Desc,
            const RHI::MResourceState& InitialState = DefaultResourcestate);

        /// <summary>
        /// Creates a readback buffer.
        /// </summary>
        [[nodiscard]] static IBuffer* Create(
            const BufferDesc&          Desc,
            const SubresourceDesc&     Subresource,
            uint64_t&                  CopyId,
            const RHI::MResourceState& InitialState = DefaultResourcestate);

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
        static constexpr auto DefaultResourcestate = RHI::MResourceState_GenericRead;
        using IBuffer::IBuffer;

        /// <summary>
        /// Creates an upload buffer.
        /// </summary>
        [[nodiscard]] static IUploadBuffer* Create(
            const BufferDesc&          Desc,
            const RHI::MResourceState& InitialState = DefaultResourcestate);

        /// <summary>
        /// Creates a readback buffer.
        /// </summary>
        [[nodiscard]] static IUploadBuffer* Create(
            const BufferDesc&          Desc,
            const SubresourceDesc&     Subresource,
            uint64_t&                  CopyId,
            const RHI::MResourceState& InitialState = DefaultResourcestate);

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
        static constexpr auto DefaultResourcestate = BitMask_Or(RHI::EResourceState::CopyDest);

        using IBuffer::IBuffer;

        /// <summary>
        /// Creates a readback buffer.
        /// </summary>
        [[nodiscard]] static IReadbackBuffer* Create(
            const BufferDesc&          Desc,
            const RHI::MResourceState& InitialState = DefaultResourcestate);

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
        static constexpr auto DefaultResourcestate = RHI::MResourceState_Common;
        using IGpuResource::IGpuResource;

        /// <summary>
        /// Creates a texture.
        /// </summary>
        [[nodiscard]] static ITexture* Create(
            const ResourceDesc&        Desc,
            const RHI::MResourceState& InitialState = DefaultResourcestate);

        /// <summary>
        /// Creates a texture.
        /// </summary>
        [[nodiscard]] static ITexture* Create(
            const ResourceDesc&              Desc,
            std::span<const SubresourceDesc> Subresources,
            uint64_t&                        CopyId,
            const RHI::MResourceState&       InitialState = DefaultResourcestate);

        /// <summary>
        /// Creates a texture.
        /// </summary>
        [[nodiscard]] static ITexture* Create(
            const TextureRawImage&     ImageData,
            uint64_t&                  CopyId,
            const RHI::MResourceState& InitialState = DefaultResourcestate);

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
            WaitForUpload();
            return m_Resource;
        }

        /// <summary>
        /// Returns the resource after it has been copied.
        /// </summary>
        [[nodiscard]] ResourcePtr Release()
        {
            WaitForUpload();
            return std::move(m_Resource);
        }

        /// <summary>
        /// Wait for copy operation to complete.
        /// </summary>
        void WaitForUpload() const
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
            const BufferDesc&          Buffer,
            const RHI::MResourceState& InitialState = IBuffer::DefaultResourcestate)
        {
            this->m_Resource.reset(IBuffer::Create(
                Buffer,
                InitialState));
        }

        SyncBufferT(
            const BufferDesc&          Desc,
            const SubresourceDesc&     Subresources,
            const RHI::MResourceState& InitialState = IBuffer::DefaultResourcestate)
        {
            this->m_Resource.reset(IBuffer::Create(
                Desc,
                Subresources,
                this->m_CopyId,
                InitialState));
        }

        SyncBufferT(
            const BufferDesc&          Desc,
            const void*                Data,
            const RHI::MResourceState& InitialState = IBuffer::DefaultResourcestate) :
            SyncBufferT(Desc,
                        SubresourceDesc{
                            .Data       = Data,
                            .RowPitch   = Desc.Size,
                            .SlicePitch = Desc.Size },
                        InitialState)
        {
        }
    };

    //

    template<bool _Owning>
    class SyncUploadBufferT : public SyncGpuResourceT<IUploadBuffer, _Owning>
    {
    public:
        using SyncGpuResourceT<IUploadBuffer, _Owning>::SyncGpuResourceT;

        SyncUploadBufferT(
            const BufferDesc&          Buffer,
            const RHI::MResourceState& InitialState = IUploadBuffer::DefaultResourcestate)
        {
            this->m_Resource.reset(IUploadBuffer::Create(
                Buffer,
                InitialState));
        }

        SyncUploadBufferT(
            const BufferDesc&          Desc,
            const SubresourceDesc&     Subresources,
            const RHI::MResourceState& InitialState = IUploadBuffer::DefaultResourcestate)
        {
            this->m_Resource.reset(IUploadBuffer::Create(
                Desc,
                Subresources,
                this->m_CopyId,
                InitialState));
        }

        SyncUploadBufferT(
            const BufferDesc&          Desc,
            const void*                Data,
            const RHI::MResourceState& InitialState = IUploadBuffer::DefaultResourcestate) :
            SyncUploadBufferT(Desc,
                              SubresourceDesc{
                                  .Data       = Data,
                                  .RowPitch   = Desc.Size,
                                  .SlicePitch = Desc.Size },
                              InitialState)
        {
        }
    };

    //

    template<bool _Owning>
    class SyncTextureT : public SyncGpuResourceT<ITexture, _Owning>
    {
    public:
        using SyncGpuResourceT<ITexture, _Owning>::SyncGpuResourceT;

        SyncTextureT(
            const TextureRawImage& ImageData,
            RHI::MResourceState    InitialState = RHI::MResourceState_Common)
        {
            this->m_Resource.reset(ITexture::Create(
                ImageData,
                this->m_CopyId,
                std::move(InitialState)));
        }

        SyncTextureT(
            const ResourceDesc& Desc,
            RHI::MResourceState InitialState = RHI::MResourceState_Common)
        {
            this->m_Resource.reset(ITexture::Create(
                Desc,
                std::move(InitialState)));
        }

        SyncTextureT(
            const ResourceDesc&              Desc,
            std::span<const SubresourceDesc> Subresources,
            RHI::MResourceState              InitialState = RHI::MResourceState_Common)
        {
            this->m_Resource.reset(ITexture::Create(
                Desc,
                Subresources,
                this->m_CopyId,
                std::move(InitialState)));
        }
    };

    //

    using SSyncBuffer = SyncBufferT<false>;
    using USyncBuffer = SyncBufferT<true>;

    using SSyncUploadBuffer = SyncUploadBufferT<false>;
    using USyncUploadBuffer = SyncUploadBufferT<true>;

    using SSyncTexture = SyncTextureT<false>;
    using USyncTexture = SyncTextureT<true>;
} // namespace Neon::RHI