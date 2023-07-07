#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/TextureLoader.hpp>
#include <Private/RHI/Dx12/Resource/LoaderHelper.hpp>
#include <Private/RHI/Dx12/Device.hpp>

namespace Neon::RHI
{
    namespace WIC
    {
        //-------------------------------------------------------------------------------------
        // WIC Pixel Format Translation Data
        //-------------------------------------------------------------------------------------
        struct WICTranslate
        {
            const GUID& wic;
            DXGI_FORMAT format;
        };

        static constexpr WICTranslate g_WICFormats[] = {
            { GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT },

            { GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT },
            { GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM },

            { GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM },
            { GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM },
            { GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM },

            { GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
            { GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM },

            { GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM },
            { GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM },

            { GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT },
            { GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT },
            { GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM },
            { GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM },

            { GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM },

            { GUID_WICPixelFormat96bppRGBFloat, DXGI_FORMAT_R32G32B32_FLOAT },
        };

        //-------------------------------------------------------------------------------------
        // WIC Pixel Format nearest conversion table
        //-------------------------------------------------------------------------------------

        struct WICConvert
        {
            const GUID& source;
            const GUID& target;
        };

        static constexpr WICConvert g_WICConvert[] = {
            // Note target GUID in this conversion table must be one of those directly supported formats (above).

            { GUID_WICPixelFormatBlackWhite, GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

            { GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

            { GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM
            { GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

            { GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf },  // DXGI_FORMAT_R16_FLOAT
            { GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT

            { GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

            { GUID_WICPixelFormat32bppBGR101010, GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

            { GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA },   // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA },   // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

            { GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA },   // DXGI_FORMAT_R16G16B16A16_UNORM
            { GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA },   // DXGI_FORMAT_R16G16B16A16_UNORM
            { GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA },  // DXGI_FORMAT_R16G16B16A16_UNORM
            { GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
            { GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

            { GUID_WICPixelFormat48bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },  // DXGI_FORMAT_R16G16B16A16_FLOAT
            { GUID_WICPixelFormat48bppBGRFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },  // DXGI_FORMAT_R16G16B16A16_FLOAT
            { GUID_WICPixelFormat64bppRGBAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
            { GUID_WICPixelFormat64bppBGRAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
            { GUID_WICPixelFormat64bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },  // DXGI_FORMAT_R16G16B16A16_FLOAT
            { GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf },        // DXGI_FORMAT_R16G16B16A16_FLOAT
            { GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf },        // DXGI_FORMAT_R16G16B16A16_FLOAT

            { GUID_WICPixelFormat128bppPRGBAFloat, GUID_WICPixelFormat128bppRGBAFloat },     // DXGI_FORMAT_R32G32B32A32_FLOAT
            { GUID_WICPixelFormat128bppRGBFloat, GUID_WICPixelFormat128bppRGBAFloat },       // DXGI_FORMAT_R32G32B32A32_FLOAT
            { GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
            { GUID_WICPixelFormat128bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat },  // DXGI_FORMAT_R32G32B32A32_FLOAT
            { GUID_WICPixelFormat32bppRGBE, GUID_WICPixelFormat128bppRGBAFloat },            // DXGI_FORMAT_R32G32B32A32_FLOAT

            { GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA },      // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA },      // DXGI_FORMAT_R16G16B16A16_UNORM
            { GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

            { GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA },           // DXGI_FORMAT_R8G8B8A8_UNORM
            { GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA },           // DXGI_FORMAT_R16G16B16A16_UNORM
            { GUID_WICPixelFormat64bppPRGBAHalf, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

            { GUID_WICPixelFormat96bppRGBFixedPoint, GUID_WICPixelFormat96bppRGBFloat }, // DXGI_FORMAT_R32G32B32_FLOAT

            // We don't support n-channel formats
        };

        static BOOL WINAPI InitializeWICFactory(PINIT_ONCE, PVOID, PVOID* ifactory) noexcept
        {
            return SUCCEEDED(CoCreateInstance(
                       CLSID_WICImagingFactory2,
                       nullptr,
                       CLSCTX_INPROC_SERVER,
                       __uuidof(IWICImagingFactory2),
                       ifactory))
                       ? TRUE
                       : FALSE;
        }

        static IWICImagingFactory2* GetWIC() noexcept
        {
            static INIT_ONCE s_initOnce = INIT_ONCE_STATIC_INIT;

            static IWICImagingFactory2* factory = nullptr;
            if (!InitOnceExecuteOnce(
                    &s_initOnce,
                    InitializeWICFactory,
                    nullptr,
                    reinterpret_cast<LPVOID*>(&factory)))
            {
                return nullptr;
            }

            return factory;
        }

        //---------------------------------------------------------------------------------
        static DXGI_FORMAT WICToDXGI(const GUID& guid) noexcept
        {
            for (size_t i = 0; i < std::size(g_WICFormats); ++i)
            {
                if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
                    return g_WICFormats[i].format;
            }

            return DXGI_FORMAT_UNKNOWN;
        }

        //---------------------------------------------------------------------------------
        static size_t WICBitsPerPixel(REFGUID targetGuid) noexcept
        {
            auto pWIC = GetWIC();
            if (!pWIC)
                return 0;

            Win32::ComPtr<IWICComponentInfo> cinfo;
            if (FAILED(pWIC->CreateComponentInfo(targetGuid, cinfo.GetAddressOf())))
                return 0;

            WICComponentType type;
            if (FAILED(cinfo->GetComponentType(&type)))
                return 0;

            if (type != WICPixelFormat)
                return 0;

            Win32::ComPtr<IWICPixelFormatInfo> pfinfo;
            if (FAILED(cinfo.As(&pfinfo)))
                return 0;

            UINT bpp;
            if (FAILED(pfinfo->GetBitsPerPixel(&bpp)))
                return 0;

            return bpp;
        }

        static HRESULT CreateTextureFromWIC(
            D3D12MA::Allocator*         allocator,
            _In_ IWICBitmapFrameDecode* frame,
            size_t                      maxsize,
            D3D12_RESOURCE_FLAGS        resFlags,
            _Outptr_ ID3D12Resource**   texture,
            _Outptr_ D3D12MA::Allocation** allocation,
            std::unique_ptr<uint8_t[]>&    decodedData,
            SubresourceDesc&               subresource) noexcept
        {
            UINT    width, height;
            HRESULT hr = frame->GetSize(&width, &height);
            if (FAILED(hr))
                return hr;

            assert(width > 0 && height > 0);

            if (maxsize > UINT32_MAX)
                return E_INVALIDARG;

            if (!maxsize)
            {
                maxsize = size_t(D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION);
            }

            UINT twidth  = width;
            UINT theight = height;
            if (width > maxsize || height > maxsize)
            {
                const float ar = static_cast<float>(height) / static_cast<float>(width);
                if (width > height)
                {
                    twidth  = static_cast<UINT>(maxsize);
                    theight = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) * ar));
                }
                else
                {
                    theight = static_cast<UINT>(maxsize);
                    twidth  = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) / ar));
                }
                assert(twidth <= maxsize && theight <= maxsize);
            }

            // Determine format
            WICPixelFormatGUID pixelFormat;
            hr = frame->GetPixelFormat(&pixelFormat);
            if (FAILED(hr))
                return hr;

            WICPixelFormatGUID convertGUID;
            memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &pixelFormat, sizeof(GUID));

            size_t bpp = 0;

            DXGI_FORMAT format = WICToDXGI(pixelFormat);
            if (format == DXGI_FORMAT_UNKNOWN)
            {
                for (size_t i = 0; i < std::size(g_WICConvert); ++i)
                {
                    if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
                    {
                        memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &g_WICConvert[i].target, sizeof(GUID));

                        format = WICToDXGI(g_WICConvert[i].target);
                        assert(format != DXGI_FORMAT_UNKNOWN);
                        bpp = WICBitsPerPixel(convertGUID);
                        break;
                    }
                }

                if (format == DXGI_FORMAT_UNKNOWN)
                {
                    /*printf("ERROR: WICTextureLoader does not support all DXGI formats (WIC GUID {%8.8lX-%4.4X-%4.4X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X}). Consider using DirectXTex.\n",
                        pixelFormat.Data1, pixelFormat.Data2, pixelFormat.Data3,
                        pixelFormat.Data4[0], pixelFormat.Data4[1], pixelFormat.Data4[2], pixelFormat.Data4[3],
                        pixelFormat.Data4[4], pixelFormat.Data4[5], pixelFormat.Data4[6], pixelFormat.Data4[7]);*/
                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }
            }
            else
            {
                bpp = WICBitsPerPixel(pixelFormat);
            }

            if (!bpp)
                return E_FAIL;

            // Allocate memory for decoded image
            const uint64_t rowBytes = (uint64_t(twidth) * uint64_t(bpp) + 7u) / 8u;
            const uint64_t numBytes = rowBytes * uint64_t(theight);

            if (rowBytes > UINT32_MAX || numBytes > UINT32_MAX)
                return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

            auto const rowPitch  = static_cast<size_t>(rowBytes);
            auto const imageSize = static_cast<size_t>(numBytes);

            decodedData.reset(new (std::nothrow) uint8_t[imageSize]);
            if (!decodedData)
                return E_OUTOFMEMORY;

            // Load image data
            if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0 && twidth == width && theight == height)
            {
                // No format conversion or resize needed
                hr = frame->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
                if (FAILED(hr))
                    return hr;
            }
            else if (twidth != width || theight != height)
            {
                // Resize
                auto pWIC = GetWIC();
                if (!pWIC)
                    return E_NOINTERFACE;

                Win32::ComPtr<IWICBitmapScaler> scaler;
                hr = pWIC->CreateBitmapScaler(scaler.GetAddressOf());
                if (FAILED(hr))
                    return hr;

                hr = scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant);
                if (FAILED(hr))
                    return hr;

                WICPixelFormatGUID pfScaler;
                hr = scaler->GetPixelFormat(&pfScaler);
                if (FAILED(hr))
                    return hr;

                if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
                {
                    // No format conversion needed
                    hr = scaler->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
                    if (FAILED(hr))
                        return hr;
                }
                else
                {
                    Win32::ComPtr<IWICFormatConverter> FC;
                    hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
                    if (FAILED(hr))
                        return hr;

                    BOOL canConvert = FALSE;
                    hr              = FC->CanConvert(pfScaler, convertGUID, &canConvert);
                    if (FAILED(hr) || !canConvert)
                    {
                        return E_UNEXPECTED;
                    }

                    hr = FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
                    if (FAILED(hr))
                        return hr;

                    hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
                    if (FAILED(hr))
                        return hr;
                }
            }
            else
            {
                // Format conversion but no resize
                auto pWIC = GetWIC();
                if (!pWIC)
                    return E_NOINTERFACE;

                Win32::ComPtr<IWICFormatConverter> FC;
                hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
                if (FAILED(hr))
                    return hr;

                BOOL canConvert = FALSE;
                hr              = FC->CanConvert(pixelFormat, convertGUID, &canConvert);
                if (FAILED(hr) || !canConvert)
                {
                    return E_UNEXPECTED;
                }

                hr = FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
                if (FAILED(hr))
                    return hr;

                hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
                if (FAILED(hr))
                    return hr;
            }

            // Count the number of mips
            const uint32_t mipCount = 1u;

            // Create texture
            D3D12_RESOURCE_DESC desc = {};
            desc.Width               = twidth;
            desc.Height              = theight;
            desc.MipLevels           = static_cast<UINT16>(mipCount);
            desc.DepthOrArraySize    = 1;
            desc.Format              = format;
            desc.SampleDesc.Count    = 1;
            desc.SampleDesc.Quality  = 0;
            desc.Flags               = resFlags;
            desc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            D3D12MA::ALLOCATION_DESC defaultHeapProperties{};
            defaultHeapProperties.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            hr = allocator->CreateResource(
                &defaultHeapProperties,
                &desc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                allocation,
                IID_PPV_ARGS(texture));

            if (FAILED(hr))
            {
                return hr;
            }

            _Analysis_assume_(texture != nullptr);

            subresource.Data       = decodedData.get();
            subresource.RowPitch   = static_cast<LONG>(rowPitch);
            subresource.SlicePitch = static_cast<LONG>(imageSize);

            return hr;
        }

        static HRESULT LoadAndDecodeWICTextureFromMemoryEx(
            D3D12MA::Allocator*       allocator,
            const uint8_t*            wicData,
            size_t                    wicDataSize,
            size_t                    maxsize,
            D3D12_RESOURCE_FLAGS      resFlags,
            _Outptr_ ID3D12Resource** texture,
            _Outptr_ D3D12MA::Allocation** allocation,
            std::unique_ptr<uint8_t[]>&    decodedData,
            std::vector<SubresourceDesc>&  initData) noexcept
        {
            if (wicDataSize > UINT32_MAX)
                return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);

            auto pWIC = GetWIC();
            if (!pWIC)
                return E_NOINTERFACE;

            // Create input stream for memory
            Win32::ComPtr<IWICStream> stream;
            HRESULT                   hr = pWIC->CreateStream(stream.GetAddressOf());
            if (FAILED(hr))
                return hr;

            hr = stream->InitializeFromMemory(const_cast<uint8_t*>(wicData), static_cast<DWORD>(wicDataSize));
            if (FAILED(hr))
                return hr;

            // Initialize WIC
            Win32::ComPtr<IWICBitmapDecoder> decoder;
            hr = pWIC->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
            if (FAILED(hr))
                return hr;

            Win32::ComPtr<IWICBitmapFrameDecode> frame;
            hr = decoder->GetFrame(0, frame.GetAddressOf());
            if (FAILED(hr))
                return hr;

            initData.resize(1);
            hr = CreateTextureFromWIC(allocator, frame.Get(), maxsize, resFlags, texture, allocation, decodedData, initData[0]);

            return hr;
        }
    } // namespace WIC

    //

    TextureLoader TextureLoader::LoadWIC(
        const uint8_t* Data,
        size_t         DataSize)
    {
        Win32::ComPtr<ID3D12Resource>      Texture;
        Win32::ComPtr<D3D12MA::Allocation> Allocation;
        std::vector<SubresourceDesc>       TextureDataToUpload;
        std::unique_ptr<uint8_t[]>         TextureDecodedData;

        ThrowIfFailed(WIC::LoadAndDecodeWICTextureFromMemoryEx(
            Dx12RenderDevice::Get()->GetAllocator()->GetMA(),
            Data,
            DataSize,
            0,
            D3D12_RESOURCE_FLAG_NONE,
            &Texture,
            &Allocation,
            TextureDecodedData,
            TextureDataToUpload));

        return TextureLoader(Texture.Get(), Allocation.Get(), TextureDataToUpload);
    }
} // namespace Neon::RHI