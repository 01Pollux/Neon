#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/TextureLoader.hpp>
#include <Private/RHI/Dx12/Resource/LoaderHelper.hpp>
#include <Private/RHI/Dx12/Device.hpp>

namespace Neon::RHI
{
    namespace DDS
    {
        //---------------------------------------------------------------------------------
        static bool IsDepthStencil(DXGI_FORMAT fmt) noexcept
        {
            switch (fmt)
            {
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            case DXGI_FORMAT_D16_UNORM:

#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
            case DXGI_FORMAT_D16_UNORM_S8_UINT:
            case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
#endif
                return true;

            default:
                return false;
            }
        }

        //--------------------------------------------------------------------------------------
        static HRESULT DxLoadTextureDataFromMemory(
            _In_reads_(ddsDataSize) const uint8_t* ddsData,
            size_t                                 ddsDataSize,
            const DDS_HEADER**                     header,
            const uint8_t**                        bitData,
            size_t*                                bitSize) noexcept
        {
            if (!header || !bitData || !bitSize)
            {
                return E_POINTER;
            }

            *bitSize = 0;

            if (ddsDataSize > UINT32_MAX)
            {
                return E_FAIL;
            }

            if (ddsDataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
            {
                return E_FAIL;
            }

            // DDS files always start with the same magic number ("DDS ")
            auto const dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData);
            if (dwMagicNumber != DDS_MAGIC)
            {
                return E_FAIL;
            }

            auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData + sizeof(uint32_t));

            // Verify header to validate DDS file
            if (hdr->size != sizeof(DDS_HEADER) ||
                hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
            {
                return E_FAIL;
            }

            // Check for DX10 extension
            bool bDXT10Header = false;
            if ((hdr->ddspf.flags & DDS_FOURCC) &&
                (MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
            {
                // Must be long enough for both headers and magic value
                if (ddsDataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)))
                {
                    return E_FAIL;
                }

                bDXT10Header = true;
            }

            // setup the pointers in the process request
            *header     = hdr;
            auto offset = sizeof(uint32_t) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0u);
            *bitData    = ddsData + offset;
            *bitSize    = ddsDataSize - offset;

            return S_OK;
        }

        //--------------------------------------------------------------------------------------
        static void AdjustPlaneResource(
            _In_ DXGI_FORMAT         fmt,
            _In_ size_t              height,
            _In_ size_t              slicePlane,
            _Inout_ SubresourceDesc& res) noexcept
        {
            switch (fmt)
            {
            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:

#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
            case DXGI_FORMAT_D16_UNORM_S8_UINT:
            case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
#endif
                if (!slicePlane)
                {
                    // Plane 0
                    res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
                }
                else
                {
                    // Plane 1
                    res.Data       = static_cast<const uint8_t*>(res.Data) + uintptr_t(res.RowPitch) * height;
                    res.SlicePitch = res.RowPitch * ((static_cast<LONG>(height) + 1) >> 1);
                }
                break;

            case DXGI_FORMAT_NV11:
                if (!slicePlane)
                {
                    // Plane 0
                    res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
                }
                else
                {
                    // Plane 1
                    res.Data       = static_cast<const uint8_t*>(res.Data) + uintptr_t(res.RowPitch) * height;
                    res.RowPitch   = (res.RowPitch >> 1);
                    res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
                }
                break;

            default:
                break;
            }
        }

        //--------------------------------------------------------------------------------------
        static HRESULT FillInitData(
            _In_ size_t                              width,
            _In_ size_t                              height,
            _In_ size_t                              depth,
            _In_ size_t                              mipCount,
            _In_ size_t                              arraySize,
            _In_ size_t                              numberOfPlanes,
            _In_ DXGI_FORMAT                         format,
            _In_ size_t                              maxsize,
            _In_ size_t                              bitSize,
            _In_reads_bytes_(bitSize) const uint8_t* bitData,
            _Out_ size_t&                            twidth,
            _Out_ size_t&                            theight,
            _Out_ size_t&                            tdepth,
            _Out_ size_t&                            skipMip,
            std::vector<SubresourceDesc>&            initData)
        {
            if (!bitData)
            {
                return E_POINTER;
            }

            skipMip = 0;
            twidth  = 0;
            theight = 0;
            tdepth  = 0;

            size_t         NumBytes = 0;
            size_t         RowBytes = 0;
            const uint8_t* pEndBits = bitData + bitSize;

            initData.clear();

            for (size_t p = 0; p < numberOfPlanes; ++p)
            {
                const uint8_t* pSrcBits = bitData;

                for (size_t j = 0; j < arraySize; j++)
                {
                    size_t w = width;
                    size_t h = height;
                    size_t d = depth;
                    for (size_t i = 0; i < mipCount; i++)
                    {
                        HRESULT hr = GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, nullptr);
                        if (FAILED(hr))
                            return hr;

                        if (NumBytes > UINT32_MAX || RowBytes > UINT32_MAX)
                            return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

                        if ((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
                        {
                            if (!twidth)
                            {
                                twidth  = w;
                                theight = h;
                                tdepth  = d;
                            }

                            SubresourceDesc res = {
                                .Data       = pSrcBits,
                                .RowPitch   = size_t(RowBytes),
                                .SlicePitch = size_t(NumBytes)
                            };

                            AdjustPlaneResource(format, h, p, res);

                            initData.emplace_back(res);
                        }
                        else if (!j)
                        {
                            // Count number of skipped mipmaps (first item only)
                            ++skipMip;
                        }

                        if (pSrcBits + (NumBytes * d) > pEndBits)
                        {
                            return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
                        }

                        pSrcBits += NumBytes * d;

                        w = w >> 1;
                        h = h >> 1;
                        d = d >> 1;
                        if (w == 0)
                        {
                            w = 1;
                        }
                        if (h == 0)
                        {
                            h = 1;
                        }
                        if (d == 0)
                        {
                            d = 1;
                        }
                    }
                }
            }

            return initData.empty() ? E_FAIL : S_OK;
        }

        //--------------------------------------------------------------------------------------
        static HRESULT CreateTextureResource(
            D3D12MA::Allocator*       allocator,
            D3D12_RESOURCE_DIMENSION  resDim,
            size_t                    width,
            size_t                    height,
            size_t                    depth,
            size_t                    mipCount,
            size_t                    arraySize,
            DXGI_FORMAT               format,
            D3D12_RESOURCE_FLAGS      resFlags,
            _Outptr_ ID3D12Resource** texture,
            _Outptr_ D3D12MA::Allocation** allocation) noexcept
        {
            HRESULT hr = E_FAIL;

            D3D12_RESOURCE_DESC desc = {};
            desc.Width               = static_cast<UINT>(width);
            desc.Height              = static_cast<UINT>(height);
            desc.MipLevels           = static_cast<UINT16>(mipCount);
            desc.DepthOrArraySize    = (resDim == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? static_cast<UINT16>(depth) : static_cast<UINT16>(arraySize);
            desc.Format              = format;
            desc.Flags               = resFlags;
            desc.SampleDesc.Count    = 1;
            desc.SampleDesc.Quality  = 0;
            desc.Dimension           = resDim;

            D3D12MA::ALLOCATION_DESC defaultHeapProperties{};
            defaultHeapProperties.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            hr = allocator->CreateResource(
                &defaultHeapProperties,
                &desc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                allocation,
                IID_PPV_ARGS(texture));
            if (SUCCEEDED(hr))
            {
                assert(texture != nullptr && *texture != nullptr);
                _Analysis_assume_(texture != nullptr && *texture != nullptr);
            }

            return hr;
        }

        //--------------------------------------------------------------------------------------
        HRESULT CreateTextureFromDDS(
            D3D12MA::Allocator*                      allocator,
            _In_ const DDS_HEADER*                   header,
            _In_reads_bytes_(bitSize) const uint8_t* bitData,
            size_t                                   bitSize,
            size_t                                   maxsize,
            D3D12_RESOURCE_FLAGS                     resFlags,
            _Outptr_ ID3D12Resource**                texture,
            _Outptr_ D3D12MA::Allocation** allocation,
            std::vector<SubresourceDesc>&  subresources)
        {
            HRESULT hr = S_OK;

            const UINT width  = header->width;
            UINT       height = header->height;
            UINT       depth  = header->depth;

            D3D12_RESOURCE_DIMENSION resDim    = D3D12_RESOURCE_DIMENSION_UNKNOWN;
            UINT                     arraySize = 1;
            DXGI_FORMAT              format    = DXGI_FORMAT_UNKNOWN;
            bool                     isCubeMap = false;

            size_t mipCount = header->mipMapCount;
            if (0 == mipCount)
            {
                mipCount = 1;
            }

            if ((header->ddspf.flags & DDS_FOURCC) &&
                (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
            {
                auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>(reinterpret_cast<const char*>(header) + sizeof(DDS_HEADER));

                arraySize = d3d10ext->arraySize;
                if (arraySize == 0)
                {
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }

                switch (d3d10ext->dxgiFormat)
                {
                case DXGI_FORMAT_AI44:
                case DXGI_FORMAT_IA44:
                case DXGI_FORMAT_P8:
                case DXGI_FORMAT_A8P8:
                    // printf("ERROR: DDSTextureLoader does not support video textures. Consider using DirectXTex instead.\n");
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

                default:
                    if (BitsPerPixel(d3d10ext->dxgiFormat) == 0)
                    {
                        // printf("ERROR: Unknown DXGI format (%u)\n", static_cast<uint32_t>(d3d10ext->dxgiFormat));
                        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                    }
                    break;
                }

                format = d3d10ext->dxgiFormat;

                switch (d3d10ext->resourceDimension)
                {
                case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                    // D3DX writes 1D textures with a fixed Height of 1
                    if ((header->flags & DDS_HEIGHT) && height != 1)
                    {
                        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    }
                    height = depth = 1;
                    break;

                case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                    if (d3d10ext->miscFlag & 0x4 /* RESOURCE_MISC_TEXTURECUBE */)
                    {
                        arraySize *= 6;
                        isCubeMap = true;
                    }
                    depth = 1;
                    break;

                case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                    if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
                    {
                        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    }

                    if (arraySize > 1)
                    {
                        // printf("ERROR: Volume textures are not texture arrays\n");
                        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                    }
                    break;

                case D3D12_RESOURCE_DIMENSION_BUFFER:
                    // printf("ERROR: Resource dimension buffer type not supported for textures\n");
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

                case D3D12_RESOURCE_DIMENSION_UNKNOWN:
                default:
                    // printf("ERROR: Unknown resource dimension (%u)\n", static_cast<uint32_t>(d3d10ext->resourceDimension));
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }

                resDim = static_cast<D3D12_RESOURCE_DIMENSION>(d3d10ext->resourceDimension);
            }
            else
            {
                format = GetDXGIFormat(header->ddspf);

                if (format == DXGI_FORMAT_UNKNOWN)
                {
                    // printf("ERROR: DDSTextureLoader does not support all legacy DDS formats. Consider using DirectXTex.\n");
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }

                if (header->flags & DDS_HEADER_FLAGS_VOLUME)
                {
                    resDim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                }
                else
                {
                    if (header->caps2 & DDS_CUBEMAP)
                    {
                        // We require all six faces to be defined
                        if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
                        {
                            // printf("ERROR: DirectX 12 does not support partial cubemaps\n");
                            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                        }

                        arraySize = 6;
                        isCubeMap = true;
                    }

                    depth  = 1;
                    resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

                    // Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
                }

                assert(BitsPerPixel(format) != 0);
            }

            // Bound sizes (for security purposes we don't trust DDS file metadata larger than the Direct3D hardware requirements)
            if (mipCount > D3D12_REQ_MIP_LEVELS)
            {
                // printf("ERROR: Too many mipmap levels defined for DirectX 12 (%zu).\n", mipCount);
                return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            }

            switch (resDim)
            {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                if ((arraySize > D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
                    (width > D3D12_REQ_TEXTURE1D_U_DIMENSION))
                {
                    // printf("ERROR: Resource dimensions too large for DirectX 12 (1D: array %u, size %u)\n", arraySize, width);
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }
                break;

            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                if (isCubeMap)
                {
                    // This is the right bound because we set arraySize to (NumCubes*6) above
                    if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                        (width > D3D12_REQ_TEXTURECUBE_DIMENSION) ||
                        (height > D3D12_REQ_TEXTURECUBE_DIMENSION))
                    {
                        // printf("ERROR: Resource dimensions too large for DirectX 12 (2D cubemap: array %u, size %u by %u)\n", arraySize, width, height);
                        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                    }
                }
                else if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                         (width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
                         (height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION))
                {
                    // printf("ERROR: Resource dimensions too large for DirectX 12 (2D: array %u, size %u by %u)\n", arraySize, width, height);
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }
                break;

            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                if ((arraySize > 1) ||
                    (width > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                    (height > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                    (depth > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
                {
                    // printf("ERROR: Resource dimensions too large for DirectX 12 (3D: array %u, size %u by %u by %u)\n", arraySize, width, height, depth);
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }
                break;

            case D3D12_RESOURCE_DIMENSION_BUFFER:
                // printf("ERROR: Resource dimension buffer type not supported for textures\n");
                return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

            default:
                // printf("ERROR: Unknown resource dimension (%u)\n", static_cast<uint32_t>(resDim));
                return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            }

            ID3D12Device* d3ddevice      = Dx12RenderDevice::Get()->GetDevice();
            const UINT    numberOfPlanes = D3D12GetFormatPlaneCount(d3ddevice, format);
            if (!numberOfPlanes)
                return E_INVALIDARG;

            if ((numberOfPlanes > 1) && IsDepthStencil(format))
            {
                // DirectX 12 uses planes for stencil, DirectX 11 does not
                return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            }

            // Create the texture
            size_t numberOfResources = (resDim == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
                                           ? 1
                                           : arraySize;
            numberOfResources *= mipCount;
            numberOfResources *= numberOfPlanes;

            if (numberOfResources > D3D12_REQ_SUBRESOURCES)
                return E_INVALIDARG;

            subresources.reserve(numberOfResources);

            size_t skipMip = 0;
            size_t twidth  = 0;
            size_t theight = 0;
            size_t tdepth  = 0;
            hr             = FillInitData(width, height, depth, mipCount, arraySize,
                                          numberOfPlanes, format,
                                          maxsize, bitSize, bitData,
                                          twidth, theight, tdepth, skipMip, subresources);

            if (SUCCEEDED(hr))
            {
                hr = CreateTextureResource(allocator, resDim, twidth, theight, tdepth, mipCount - skipMip, arraySize,
                                           format, resFlags, texture, allocation);

                if (FAILED(hr) && !maxsize && (mipCount > 1))
                {
                    subresources.clear();

                    maxsize = static_cast<size_t>(
                        (resDim == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
                            ? D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
                            : D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION);

                    hr = FillInitData(width, height, depth, mipCount, arraySize,
                                      numberOfPlanes, format,
                                      maxsize, bitSize, bitData,
                                      twidth, theight, tdepth, skipMip, subresources);
                    if (SUCCEEDED(hr))
                    {
                        hr = CreateTextureResource(allocator, resDim, twidth, theight, tdepth, mipCount - skipMip, arraySize,
                                                   format, resFlags, texture, allocation);
                    }
                }
            }

            if (FAILED(hr))
            {
                subresources.clear();
            }

            return hr;
        }

        //--------------------------------------------------------------------------------------
        static HRESULT LoadDDSTextureFromMemoryEx(
            D3D12MA::Allocator*  allocator,
            const uint8_t*       ddsData,
            size_t               ddsDataSize,
            size_t               maxsize,
            D3D12_RESOURCE_FLAGS resFlags,
            ID3D12Resource**     texture,
            _Outptr_ D3D12MA::Allocation** allocation,
            std::vector<SubresourceDesc>&  subresources)
        {
            if (texture)
            {
                *texture = nullptr;
            }

            // Validate DDS file in memory
            const DDS_HEADER* header  = nullptr;
            const uint8_t*    bitData = nullptr;
            size_t            bitSize = 0;

            HRESULT hr = LoadTextureDataFromMemory(ddsData,
                                                   ddsDataSize,
                                                   &header,
                                                   &bitData,
                                                   &bitSize);
            if (FAILED(hr))
            {
                return hr;
            }

            return CreateTextureFromDDS(
                allocator,
                header, bitData, bitSize, maxsize,
                resFlags, texture, allocation, subresources);
        }
    } // namespace DDS

    //

    TextureLoader TextureLoader::LoadDDS(
        const uint8_t* Data,
        size_t         DataSize)
    {
        Win32::ComPtr<ID3D12Resource>      Texture;
        Win32::ComPtr<D3D12MA::Allocation> Allocation;
        std::vector<SubresourceDesc>       TextureDataToUpload;
        std::unique_ptr<uint8_t[]>         TextureDecodedData;

        ThrowIfFailed(DDS::LoadDDSTextureFromMemoryEx(
            Dx12RenderDevice::Get()->GetAllocator()->GetMA(),
            Data,
            DataSize,
            0,
            D3D12_RESOURCE_FLAG_NONE,
            &Texture,
            &Allocation,
            TextureDataToUpload));

        return TextureLoader(Texture.Get(), Allocation.Get(), TextureDataToUpload);
    }
} // namespace Neon::RHI