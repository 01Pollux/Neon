#pragma once

#include <Math/Vector.hpp>
#include <Asio/Coroutines.hpp>

#include <RHI/Resource/Common.hpp>
#include <RHI/Swapchain.hpp>

#include <variant>
#include <span>

namespace Neon::RHI
{
    class ISwapchain;
    class ICommandQueue;
    class IGpuResource;

    struct DescriptorHeapHandle;

    //

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
        size_t          Width,
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

    /// <summary>
    /// Computes buffer's subresource desc
    /// </summary>
    [[nodiscard]] static SubresourceDesc ComputeSubresource(
        const void* Data,
        size_t      Size)
    {
        return SubresourceDesc{
            .Data       = Data,
            .RowPitch   = Size,
            .SlicePitch = Size
        };
    }

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

    //

    class IGpuResource
    {
    public:
        static constexpr auto DefaultResourcestate         = RHI::MResourceState_Common;
        static constexpr auto DefaultUploadResourcestate   = RHI::MResourceState_GenericRead;
        static constexpr auto DefaultReadbackResourcestate = BitMask_Or(RHI::EResourceState::CopyDest);

        struct InitDesc
        {
            const wchar_t*                   Name         = nullptr;
            std::span<const SubresourceDesc> Subresources = {};
            std::future<void>*               CopyTask     = nullptr;
            RHI::MResourceState              InitialState = DefaultResourcestate;
        };

        /// <summary>
        /// Creates a resource.
        /// </summary>
        [[nodiscard]] static IGpuResource* Create(
            const ResourceDesc& Desc,
            const InitDesc&     Init = {});

        /// <summary>
        /// Creates a resource.
        /// Subresources are not used in here
        /// </summary>
        [[nodiscard]] static IGpuResource* Create(
            const TextureRawImage& ImageData,
            const InitDesc&        Init = {});

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
        virtual std::future<void> CopyFrom(
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            std::optional<RHI::MResourceState> NextState = std::nullopt) = 0;

        /// <summary>
        /// Copy the subresource to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t               FirstSubresource,
            const SubresourceDesc& Subresource)
        {
            std::array Subresources = { Subresource };
            return CopyFrom(FirstSubresource, Subresources);
        }

        /// <summary>
        /// Get desc of the resource.
        /// </summary>
        [[nodiscard]] const ResourceDesc& GetDesc() const
        {
            return m_Desc;
        }

    public:
        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] size_t GetSize() const
        {
            return m_Desc.Width;
        }

        /// <summary>
        /// Get resource handle in gpu.
        /// </summary>
        [[nodiscard]] virtual GpuResourceHandle GetHandle(
            size_t Offset = 0) const = 0;

    public:
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

        /// <summary>
        /// Map and read data from the buffer, then unmap.
        /// </summary>
        void Read(
            size_t Offset,
            void*  Data,
            size_t Size)
        {
            auto Mapped = Map<uint8_t>(Offset);
            std::copy_n(Mapped, Size, static_cast<uint8_t*>(Data));
            Unmap();
        }

    public:
        /// Returns the dimensions of the texture.
        /// </summary>
        [[nodiscard]] Vector3I GetDimensions() const
        {
            return {
                int(m_Desc.Width),
                int(m_Desc.Height),
                int(m_Desc.Depth)
            };
        }

        /// <summary>
        /// Returns the number of mip levels in the texture.
        /// </summary>
        [[nodiscard]] uint16_t GetMipLevels() const
        {
            return m_Desc.MipLevels;
        }

        /// <summary>
        /// Get number of subresources in texture
        /// </summary>
        [[nodiscard]] uint32_t GetSubResourceCount() const
        {
            return uint32_t(m_Desc.MipLevels) * m_Desc.Depth;
        }

        /// <summary>
        /// Get subresource index from plane, array and mip index
        /// </summary>
        [[nodiscard]] uint32_t GetSubresourceIndex(
            uint32_t PlaneIndex,
            uint32_t ArrayIndex,
            uint32_t MipIndex) const
        {
            return GetSubresourceIndex(
                PlaneIndex,
                ArrayIndex,
                m_Desc.Depth,
                MipIndex,
                m_Desc.MipLevels);
        }

        /// <summary>
        /// Get subresource index from plane, array and mip index
        /// </summary>
        [[nodiscard]] static constexpr uint32_t GetSubresourceIndex(
            uint32_t PlaneIndex,
            uint32_t ArrayIndex,
            uint32_t ArraySize,
            uint32_t MipIndex,
            uint32_t MipSize) noexcept
        {
            return MipIndex +
                   ArrayIndex * MipSize +
                   PlaneIndex * ArraySize * MipSize;
        }

    public:
        /// <summary>
        /// Get default texture for specified type.
        /// </summary>
        static [[nodiscard]] const Ptr<IGpuResource>& GetDefaultTexture(
            DefaultTextures Type);

    protected:
        ResourceDesc m_Desc;
    };

    //

    class GpuBuffer
    {
    public:
        static constexpr auto DefaultResourcestate = RHI::MResourceState_Common;
        GpuBuffer(
            IGpuResource* Resource = nullptr) :
            m_Resource(Resource)
        {
        }

        /// <summary>
        /// Get the resource
        /// </summary>
        [[nodiscard]] IGpuResource* Get() const noexcept
        {
            return m_Resource;
        }

        operator IGpuResource*() const noexcept
        {
            return m_Resource;
        }

        /// <summary>
        /// Query the footprint of the resource.
        /// </summary>
        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               LinearSize) const
        {
            m_Resource->QueryFootprint(
                FirstSubresource,
                SubresourceCount,
                Offset,
                OutFootprint,
                NumRows,
                RowSizeInBytes,
                LinearSize);
        }

        /// <summary>
        /// Copy the subresources to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            std::optional<RHI::MResourceState> NextState = std::nullopt)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                std::move(Subresources),
                std::move(NextState));
        }

        /// <summary>
        /// Copy the subresource to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t               FirstSubresource,
            const SubresourceDesc& Subresource)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                Subresource);
        }

        /// <summary>
        /// Get desc of the resource.
        /// </summary>
        [[nodiscard]] const ResourceDesc& GetDesc() const
        {
            return m_Resource->GetDesc();
        }

    public:
        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] size_t GetSize() const
        {
            return m_Resource->GetSize();
        }

        /// <summary>
        /// Get resource handle in gpu.
        /// </summary>
        [[nodiscard]] GpuResourceHandle GetHandle(
            size_t Offset = 0) const
        {
            return m_Resource->GetHandle(Offset);
        }

    protected:
        IGpuResource* m_Resource;
    };

    //

    class GpuUploadBuffer
    {
    public:
        static constexpr auto DefaultResourcestate = RHI::MResourceState_Common;
        GpuUploadBuffer(
            IGpuResource* Resource = nullptr) :
            m_Resource(Resource)
        {
        }

        /// <summary>
        /// Get the resource
        /// </summary>
        [[nodiscard]] IGpuResource* Get() const noexcept
        {
            return m_Resource;
        }

        operator IGpuResource*() const noexcept
        {
            return m_Resource;
        }

        /// <summary>
        /// Query the footprint of the resource.
        /// </summary>
        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               LinearSize) const
        {
            m_Resource->QueryFootprint(
                FirstSubresource,
                SubresourceCount,
                Offset,
                OutFootprint,
                NumRows,
                RowSizeInBytes,
                LinearSize);
        }

        /// <summary>
        /// Copy the subresources to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            std::optional<RHI::MResourceState> NextState = std::nullopt)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                std::move(Subresources),
                std::move(NextState));
        }

        /// <summary>
        /// Copy the subresource to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t               FirstSubresource,
            const SubresourceDesc& Subresource)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                Subresource);
        }

        /// <summary>
        /// Get desc of the resource.
        /// </summary>
        [[nodiscard]] const ResourceDesc& GetDesc() const
        {
            return m_Resource->GetDesc();
        }

    public:
        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] size_t GetSize() const
        {
            return m_Resource->GetSize();
        }

        /// <summary>
        /// Get resource handle in gpu.
        /// </summary>
        [[nodiscard]] GpuResourceHandle GetHandle(
            size_t Offset = 0) const
        {
            return m_Resource->GetHandle(Offset);
        }

    public:
        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        [[nodiscard]] uint8_t* Map()
        {
            return m_Resource->Map();
        }

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        void Unmap()
        {
            return m_Resource->Unmap();
        }

        /// <summary>
        /// Maps the buffer to the specified type.
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] _Ty* Map(
            size_t Offset = 0)
        {
            return m_Resource->Map<_Ty>(Offset);
        }

        /// <summary>
        /// Map and write data to the buffer, then unmap.
        /// </summary>
        void Write(
            size_t      Offset,
            const void* Data,
            size_t      Size)
        {
            m_Resource->Write(Offset, Data, Size);
        }

    protected:
        IGpuResource* m_Resource;
    };

    //

    class GpuReadbackBuffer
    {
    public:
        static constexpr auto DefaultResourcestate = RHI::MResourceState_Common;
        GpuReadbackBuffer(
            IGpuResource* Resource = nullptr) :
            m_Resource(Resource)
        {
        }

        /// <summary>
        /// Get the resource
        /// </summary>
        [[nodiscard]] IGpuResource* Get() const noexcept
        {
            return m_Resource;
        }

        operator IGpuResource*() const noexcept
        {
            return m_Resource;
        }

        /// <summary>
        /// Query the footprint of the resource.
        /// </summary>
        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               LinearSize) const
        {
            m_Resource->QueryFootprint(
                FirstSubresource,
                SubresourceCount,
                Offset,
                OutFootprint,
                NumRows,
                RowSizeInBytes,
                LinearSize);
        }

        /// <summary>
        /// Copy the subresources to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            std::optional<RHI::MResourceState> NextState = std::nullopt)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                std::move(Subresources),
                std::move(NextState));
        }

        /// <summary>
        /// Copy the subresource to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t               FirstSubresource,
            const SubresourceDesc& Subresource)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                Subresource);
        }

        /// <summary>
        /// Get desc of the resource.
        /// </summary>
        [[nodiscard]] const ResourceDesc& GetDesc() const
        {
            return m_Resource->GetDesc();
        }

    public:
        /// <summary>
        /// Get the size of the buffer in bytes.
        /// </summary>
        [[nodiscard]] size_t GetSize() const
        {
            return m_Resource->GetSize();
        }

        /// <summary>
        /// Get resource handle in gpu.
        /// </summary>
        [[nodiscard]] GpuResourceHandle GetHandle(
            size_t Offset = 0) const
        {
            return m_Resource->GetHandle(Offset);
        }

    public:
        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        [[nodiscard]] uint8_t* Map()
        {
            return m_Resource->Map();
        }

        /// <summary>
        /// Makes the buffer available for reading/writing by the CPU.
        /// </summary>
        void Unmap()
        {
            return m_Resource->Unmap();
        }

        /// <summary>
        /// Maps the buffer to the specified type.
        /// </summary>
        template<typename _Ty>
        [[nodiscard]] _Ty* Map(
            size_t Offset = 0)
        {
            return m_Resource->Map<_Ty>(Offset);
        }

        /// <summary>
        /// Map and read data from the buffer, then unmap.
        /// </summary>
        void Read(
            size_t Offset,
            void*  Data,
            size_t Size)
        {
            m_Resource->Read(Offset, Data, Size);
        }

    protected:
        IGpuResource* m_Resource;
    };

    //

    class GpuTexture
    {
    public:
        static constexpr auto DefaultResourcestate = RHI::MResourceState_Common;
        GpuTexture(
            IGpuResource* Resource = nullptr) :
            m_Resource(Resource)
        {
        }

        /// <summary>
        /// Get the resource
        /// </summary>
        [[nodiscard]] IGpuResource* Get() const noexcept
        {
            return m_Resource;
        }

        operator IGpuResource*() const noexcept
        {
            return m_Resource;
        }

        /// <summary>
        /// Query the footprint of the resource.
        /// </summary>
        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               LinearSize) const
        {
            m_Resource->QueryFootprint(
                FirstSubresource,
                SubresourceCount,
                Offset,
                OutFootprint,
                NumRows,
                RowSizeInBytes,
                LinearSize);
        }

        /// <summary>
        /// Copy the subresources to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            std::optional<RHI::MResourceState> NextState = std::nullopt)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                std::move(Subresources),
                std::move(NextState));
        }

        /// <summary>
        /// Copy the subresource to the resource
        /// </summary>
        std::future<void> CopyFrom(
            uint32_t               FirstSubresource,
            const SubresourceDesc& Subresource)
        {
            return m_Resource->CopyFrom(
                FirstSubresource,
                Subresource);
        }

        /// <summary>
        /// Get desc of the resource.
        /// </summary>
        [[nodiscard]] const ResourceDesc& GetDesc() const
        {
            return m_Resource->GetDesc();
        }

    public:
        /// Returns the dimensions of the texture.
        /// </summary>
        [[nodiscard]] Vector3I GetDimensions() const
        {
            return m_Resource->GetDimensions();
        }

        /// <summary>
        /// Returns the number of mip levels in the texture.
        /// </summary>
        [[nodiscard]] uint16_t GetMipLevels() const
        {
            return m_Resource->GetMipLevels();
        }

        /// <summary>
        /// Get number of subresources in texture
        /// </summary>
        [[nodiscard]] uint32_t GetSubResourceCount() const
        {
            return m_Resource->GetSubResourceCount();
        }

        /// <summary>
        /// Get subresource index from plane, array and mip index
        /// </summary>
        [[nodiscard]] uint32_t GetSubresourceIndex(
            uint32_t PlaneIndex,
            uint32_t ArrayIndex,
            uint32_t MipIndex) const
        {
            return m_Resource->GetSubresourceIndex(PlaneIndex, ArrayIndex, MipIndex);
        }

    public:
        /// <summary>
        /// Get default texture for specified type.
        /// </summary>
        static [[nodiscard]] const Ptr<IGpuResource>& GetDefaultTexture(
            DefaultTextures Type)
        {
            return IGpuResource::GetDefaultTexture(Type);
        }

    protected:
        IGpuResource* m_Resource;
    };

    //

    enum class SyncGpuResourcePtrType : uint8_t
    {
        Reference,
        SharedPtr
    };

    /// <summary>
    /// Upload resource asynchronously and wait for it to be ready when accessed.
    /// </summary>
    template<typename ResourcePtr, typename TaskPtr>
    class SyncGpuResourceT
    {
    public:
        SyncGpuResourceT() = default;

        SyncGpuResourceT(
            ResourcePtr       Resource,
            std::future<void> CopyTask) :
            m_Resource(std::move(Resource))
        {
            SetTask(std::move(CopyTask));
        }

        SyncGpuResourceT(
            ResourcePtr Resource) :
            m_Resource(std::move(Resource))
        {
        }

        SyncGpuResourceT(
            const ResourceDesc&              Desc,
            std::span<const SubresourceDesc> Subresources = {},
            const wchar_t*                   Name         = nullptr,
            RHI::MResourceState              InitialState = IGpuResource::DefaultResourcestate)
        {
            std::future<void>      Future;
            IGpuResource::InitDesc Init{
                .Name         = Name,
                .Subresources = Subresources,
                .CopyTask     = Subresources.empty() ? nullptr : &Future,
                .InitialState = InitialState
            };

            m_Resource.reset(IGpuResource::Create(Desc, Init));
            if (Init.CopyTask) [[likely]]
            {
                SetTask(std::move(Future));
            }
        }

        SyncGpuResourceT(
            const ResourceDesc&    Desc,
            const SubresourceDesc& Subresource,
            const wchar_t*         Name         = nullptr,
            RHI::MResourceState    InitialState = IGpuResource::DefaultResourcestate)
        {
            std::future<void>      Future;
            IGpuResource::InitDesc Init{
                .Name         = Name,
                .Subresources = { &Subresource, 1 },
                .CopyTask     = Subresource.Data ? &Future : nullptr,
                .InitialState = InitialState
            };

            m_Resource.reset(IGpuResource::Create(Desc, Init));
            if (Init.CopyTask) [[likely]]
            {
                SetTask(std::move(Future));
            }
        }

        SyncGpuResourceT(
            const TextureRawImage& ImageData,
            const wchar_t*         Name         = nullptr,
            RHI::MResourceState    InitialState = IGpuResource::DefaultResourcestate)
        {
            std::future<void>      Future;
            IGpuResource::InitDesc Init{
                .Name         = Name,
                .CopyTask     = &Future,
                .InitialState = InitialState
            };

            m_Resource.reset(IGpuResource::Create(ImageData, Init));
            SetTask(std::move(Future));
        }

        static SyncGpuResourceT Buffer(
            size_t              Stride,
            size_t              Count,
            const void*         Data,
            const wchar_t*      Name         = nullptr,
            RHI::MResourceFlags Flags        = {},
            RHI::MResourceState InitialState = IGpuResource::DefaultResourcestate)
        {
            return SyncGpuResourceT(
                ResourceDesc::Buffer(Stride * Count, Flags),
                ComputeSubresource(Data, Stride * Count),
                Name,
                InitialState);
        }

    public:
        /// <summary>
        /// Returns the resource after it has been copied.
        /// </summary>
        [[nodiscard]] const ResourcePtr&
        operator->() const
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
            if (m_CopyTask.valid() && m_Resource) [[unlikely]]
            {
                m_CopyTask.get();
            }
        }

    protected:
        /// <summary>
        /// Set copy operation task
        /// </summary>
        void SetTask(
            std::future<void> CopyTask)
        {
            if constexpr (std::is_same_v<TaskPtr, std::future<void>>)
            {
                m_CopyTask = std::move(CopyTask);
            }
            else
            {
                m_CopyTask = CopyTask.share();
            }
        }

    protected:
        ResourcePtr     m_Resource;
        mutable TaskPtr m_CopyTask;
    };

    using SSyncGpuResource = SyncGpuResourceT<Ptr<IGpuResource>, std::shared_future<void>>;
    using USyncGpuResource = SyncGpuResourceT<UPtr<IGpuResource>, std::future<void>>;
} // namespace Neon::RHI