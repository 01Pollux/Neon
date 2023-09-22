#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>

namespace Neon::RHI
{
    [[nodiscard]] static constexpr auto GetResourceStateList() noexcept
    {
        return std::array{
            std::pair{ D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, EResourceState::VertexAndConstantBuffer },
            std::pair{ D3D12_RESOURCE_STATE_INDEX_BUFFER, EResourceState::IndexBuffer },
            std::pair{ D3D12_RESOURCE_STATE_RENDER_TARGET, EResourceState::RenderTarget },
            std::pair{ D3D12_RESOURCE_STATE_UNORDERED_ACCESS, EResourceState::UnorderedAccess },
            std::pair{ D3D12_RESOURCE_STATE_DEPTH_WRITE, EResourceState::DepthWrite },
            std::pair{ D3D12_RESOURCE_STATE_DEPTH_READ, EResourceState::DepthRead },
            std::pair{ D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, EResourceState::NonPixelShaderResource },
            std::pair{ D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, EResourceState::PixelShaderResource },
            std::pair{ D3D12_RESOURCE_STATE_STREAM_OUT, EResourceState::StreamOut },
            std::pair{ D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, EResourceState::IndirectArgument },
            std::pair{ D3D12_RESOURCE_STATE_COPY_DEST, EResourceState::CopyDest },
            std::pair{ D3D12_RESOURCE_STATE_COPY_SOURCE, EResourceState::CopySource },
            std::pair{ D3D12_RESOURCE_STATE_RESOLVE_DEST, EResourceState::ResolveDest },
            std::pair{ D3D12_RESOURCE_STATE_RESOLVE_SOURCE, EResourceState::ResolveSource },
            std::pair{ D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, EResourceState::RaytracingAccelerationStructure },
            std::pair{ D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE, EResourceState::ShadingRateSource },
            std::pair{ D3D12_RESOURCE_STATE_PREDICATION, EResourceState::Predication },
            std::pair{ D3D12_RESOURCE_STATE_VIDEO_DECODE_READ, EResourceState::VideoDecodeRead },
            std::pair{ D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE, EResourceState::VideoDecodeWrite },
            std::pair{ D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ, EResourceState::VideoProcessRead },
            std::pair{ D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE, EResourceState::VideoProcessWrite },
            std::pair{ D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ, EResourceState::VideoEncodeRead },
            std::pair{ D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE, EResourceState::VideoEncodeWrite }
        };
    }

    [[nodiscard]] static constexpr auto GetResourceFlagList() noexcept
    {
        return std::array{
            std::pair{ D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, EResourceFlags::AllowRenderTarget },
            std::pair{ D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, EResourceFlags::AllowDepthStencil },
            std::pair{ D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, EResourceFlags::AllowUnorderedAccess },
            std::pair{ D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE, EResourceFlags::DenyShaderResource },
            std::pair{ D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER, EResourceFlags::AllowCrossAdapter },
            std::pair{ D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS, EResourceFlags::AllowSimultaneousAccess },
            std::pair{ D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE, EResourceFlags::RayTracingAccelerationStruct }
        };
    }

    //

    ID3D12Resource* GetDx12Resource(IGpuResource* Resource)
    {
        return static_cast<Dx12GpuResource*>(Resource)->GetResource();
    }

    D3D12_RESOURCE_STATES CastResourceStates(
        EResourceState States)
    {
        for (auto& State : GetResourceStateList())
        {
            if (States == State.second)
            {
                return State.first;
            }
        }
        return D3D12_RESOURCE_STATE_COMMON;
    }

    D3D12_RESOURCE_STATES CastResourceStates(
        const MResourceState& States)
    {
        D3D12_RESOURCE_STATES Res = D3D12_RESOURCE_STATE_COMMON;
        for (auto& State : GetResourceStateList())
        {
            if (States.Test(State.second))
            {
                Res |= State.first;
            }
        }
        return Res;
    }

    MResourceState CastResourceStates(
        D3D12_RESOURCE_STATES States)
    {
        MResourceState Res;
        for (auto& State : GetResourceStateList())
        {
            if ((States & State.first) == State.first)
            {
                Res.Set(State.second);
            }
        }
        return Res;
    }

    //

    DXGI_FORMAT CastFormat(
        EResourceFormat Format)
    {
        switch (Format)
        {
        case EResourceFormat::Unknown:
            return DXGI_FORMAT_UNKNOWN;
        case EResourceFormat::R32G32B32A32_Typeless:
            return DXGI_FORMAT_R32G32B32A32_TYPELESS;
        case EResourceFormat::R32G32B32A32_Float:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case EResourceFormat::R32G32B32A32_UInt:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case EResourceFormat::R32G32B32A32_SInt:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case EResourceFormat::R32G32B32_Typeless:
            return DXGI_FORMAT_R32G32B32_TYPELESS;
        case EResourceFormat::R32G32B32_Float:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case EResourceFormat::R32G32B32_UInt:
            return DXGI_FORMAT_R32G32B32_UINT;
        case EResourceFormat::R32G32B32_SInt:
            return DXGI_FORMAT_R32G32B32_SINT;
        case EResourceFormat::R16G16B16A16_Typeless:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case EResourceFormat::R16G16B16A16_Float:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case EResourceFormat::R16G16B16A16_UNorm:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case EResourceFormat::R16G16B16A16_UInt:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case EResourceFormat::R16G16B16A16_SNorm:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case EResourceFormat::R16G16B16A16_SInt:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case EResourceFormat::R32G32_Typeless:
            return DXGI_FORMAT_R32G32_TYPELESS;
        case EResourceFormat::R32G32_Float:
            return DXGI_FORMAT_R32G32_FLOAT;
        case EResourceFormat::R32G32_UInt:
            return DXGI_FORMAT_R32G32_UINT;
        case EResourceFormat::R32G32_SInt:
            return DXGI_FORMAT_R32G32_SINT;
        case EResourceFormat::R32G8X24_Typeless:
            return DXGI_FORMAT_R32G8X24_TYPELESS;
        case EResourceFormat::D32_Float_S8X24_UInt:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case EResourceFormat::R32_Float_X8X24_Typeless:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        case EResourceFormat::X32_Typeless_G8X24_UInt:
            return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        case EResourceFormat::R10G10B10A2_Typeless:
            return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case EResourceFormat::R10G10B10A2_UNorm:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case EResourceFormat::R10G10B10A2_UInt:
            return DXGI_FORMAT_R10G10B10A2_UINT;
        case EResourceFormat::R11G11B10_Float:
            return DXGI_FORMAT_R11G11B10_FLOAT;
        case EResourceFormat::R8G8B8A8_Typeless:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case EResourceFormat::R8G8B8A8_UNorm:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case EResourceFormat::R8G8B8A8_UNorm_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case EResourceFormat::R8G8B8A8_UInt:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case EResourceFormat::R8G8B8A8_SNorm:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case EResourceFormat::R8G8B8A8_SInt:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case EResourceFormat::R16G16_Typeless:
            return DXGI_FORMAT_R16G16_TYPELESS;
        case EResourceFormat::R16G16_Float:
            return DXGI_FORMAT_R16G16_FLOAT;
        case EResourceFormat::R16G16_UNorm:
            return DXGI_FORMAT_R16G16_UNORM;
        case EResourceFormat::R16G16_UInt:
            return DXGI_FORMAT_R16G16_UINT;
        case EResourceFormat::R16G16_SNorm:
            return DXGI_FORMAT_R16G16_SNORM;
        case EResourceFormat::R16G16_SInt:
            return DXGI_FORMAT_R16G16_SINT;
        case EResourceFormat::R32_Typeless:
            return DXGI_FORMAT_R32_TYPELESS;
        case EResourceFormat::D32_Float:
            return DXGI_FORMAT_D32_FLOAT;
        case EResourceFormat::R32_Float:
            return DXGI_FORMAT_R32_FLOAT;
        case EResourceFormat::R32_UInt:
            return DXGI_FORMAT_R32_UINT;
        case EResourceFormat::R32_SInt:
            return DXGI_FORMAT_R32_SINT;
        case EResourceFormat::R24G8_Typeless:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case EResourceFormat::D24_UNorm_S8_UInt:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case EResourceFormat::R24_UNorm_X8_Typeless:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case EResourceFormat::X24_Typeless_G8_UInt:
            return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        case EResourceFormat::R8G8_Typeless:
            return DXGI_FORMAT_R8G8_TYPELESS;
        case EResourceFormat::R8G8_UNorm:
            return DXGI_FORMAT_R8G8_UNORM;
        case EResourceFormat::R8G8_UInt:
            return DXGI_FORMAT_R8G8_UINT;
        case EResourceFormat::R8G8_SNorm:
            return DXGI_FORMAT_R8G8_SNORM;
        case EResourceFormat::R8G8_SInt:
            return DXGI_FORMAT_R8G8_SINT;
        case EResourceFormat::R16_Typeless:
            return DXGI_FORMAT_R16_TYPELESS;
        case EResourceFormat::R16_Float:
            return DXGI_FORMAT_R16_FLOAT;
        case EResourceFormat::D16_UNorm:
            return DXGI_FORMAT_D16_UNORM;
        case EResourceFormat::R16_UNorm:
            return DXGI_FORMAT_R16_UNORM;
        case EResourceFormat::R16_UInt:
            return DXGI_FORMAT_R16_UINT;
        case EResourceFormat::R16_SNorm:
            return DXGI_FORMAT_R16_SNORM;
        case EResourceFormat::R16_SInt:
            return DXGI_FORMAT_R16_SINT;
        case EResourceFormat::R8_Typeless:
            return DXGI_FORMAT_R8_TYPELESS;
        case EResourceFormat::R8_UNorm:
            return DXGI_FORMAT_R8_UNORM;
        case EResourceFormat::R8_UInt:
            return DXGI_FORMAT_R8_UINT;
        case EResourceFormat::R8_SNorm:
            return DXGI_FORMAT_R8_SNORM;
        case EResourceFormat::R8_SInt:
            return DXGI_FORMAT_R8_SINT;
        case EResourceFormat::A8_UNorm:
            return DXGI_FORMAT_A8_UNORM;
        case EResourceFormat::R1_UNorm:
            return DXGI_FORMAT_R1_UNORM;
        case EResourceFormat::R9G9B9E5_SharedExp:
            return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
        case EResourceFormat::R8G8_B8G8_UNorm:
            return DXGI_FORMAT_R8G8_B8G8_UNORM;
        case EResourceFormat::G8R8_G8B8_UNorm:
            return DXGI_FORMAT_G8R8_G8B8_UNORM;
        case EResourceFormat::BC1_Typeless:
            return DXGI_FORMAT_BC1_TYPELESS;
        case EResourceFormat::BC1_UNorm:
            return DXGI_FORMAT_BC1_UNORM;
        case EResourceFormat::BC1_UNorm_SRGB:
            return DXGI_FORMAT_BC1_UNORM_SRGB;
        case EResourceFormat::BC2_Typeless:
            return DXGI_FORMAT_BC2_TYPELESS;
        case EResourceFormat::BC2_UNorm:
            return DXGI_FORMAT_BC2_UNORM;
        case EResourceFormat::BC2_UNorm_SRGB:
            return DXGI_FORMAT_BC2_UNORM_SRGB;
        case EResourceFormat::BC3_Typeless:
            return DXGI_FORMAT_BC3_TYPELESS;
        case EResourceFormat::BC3_UNorm:
            return DXGI_FORMAT_BC3_UNORM;
        case EResourceFormat::BC3_UNorm_SRGB:
            return DXGI_FORMAT_BC3_UNORM_SRGB;
        case EResourceFormat::BC4_Typeless:
            return DXGI_FORMAT_BC4_TYPELESS;
        case EResourceFormat::BC4_UNorm:
            return DXGI_FORMAT_BC4_UNORM;
        case EResourceFormat::BC4_SNorm:
            return DXGI_FORMAT_BC4_SNORM;
        case EResourceFormat::BC5_Typeless:
            return DXGI_FORMAT_BC5_TYPELESS;
        case EResourceFormat::BC5_UNorm:
            return DXGI_FORMAT_BC5_UNORM;
        case EResourceFormat::BC5_SNorm:
            return DXGI_FORMAT_BC5_SNORM;
        case EResourceFormat::B5G6R5_UNorm:
            return DXGI_FORMAT_B5G6R5_UNORM;
        case EResourceFormat::B5G5R5A1_UNorm:
            return DXGI_FORMAT_B5G5R5A1_UNORM;
        case EResourceFormat::B8G8R8A8_UNorm:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case EResourceFormat::B8G8R8X8_UNorm:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        case EResourceFormat::R10G10B10_XR_Bias_A2_UNorm:
            return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
        case EResourceFormat::B8G8R8A8_Typeless:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        case EResourceFormat::B8G8R8A8_UNorm_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case EResourceFormat::B8G8R8X8_Typeless:
            return DXGI_FORMAT_B8G8R8X8_TYPELESS;
        case EResourceFormat::B8G8R8X8_UNorm_SRGB:
            return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
        case EResourceFormat::BC6H_Typeless:
            return DXGI_FORMAT_BC6H_TYPELESS;
        case EResourceFormat::BC6H_UF16:
            return DXGI_FORMAT_BC6H_UF16;
        case EResourceFormat::BC6H_SF16:
            return DXGI_FORMAT_BC6H_SF16;
        case EResourceFormat::BC7_Typeless:
            return DXGI_FORMAT_BC7_TYPELESS;
        case EResourceFormat::BC7_UNorm:
            return DXGI_FORMAT_BC7_UNORM;
        case EResourceFormat::BC7_UNorm_SRGB:
            return DXGI_FORMAT_BC7_UNORM_SRGB;
        case EResourceFormat::AYUV:
            return DXGI_FORMAT_AYUV;
        case EResourceFormat::Y410:
            return DXGI_FORMAT_Y410;
        case EResourceFormat::Y416:
            return DXGI_FORMAT_Y416;
        case EResourceFormat::NV12:
            return DXGI_FORMAT_NV12;
        case EResourceFormat::P010:
            return DXGI_FORMAT_P010;
        case EResourceFormat::P016:
            return DXGI_FORMAT_P016;
        case EResourceFormat::Opaque_420:
            return DXGI_FORMAT_420_OPAQUE;
        case EResourceFormat::YUY2:
            return DXGI_FORMAT_YUY2;
        case EResourceFormat::Y210:
            return DXGI_FORMAT_Y210;
        case EResourceFormat::Y216:
            return DXGI_FORMAT_Y216;
        case EResourceFormat::NV11:
            return DXGI_FORMAT_NV11;
        case EResourceFormat::AI44:
            return DXGI_FORMAT_AI44;
        case EResourceFormat::IA44:
            return DXGI_FORMAT_IA44;
        case EResourceFormat::P8:
            return DXGI_FORMAT_P8;
        case EResourceFormat::A8P8:
            return DXGI_FORMAT_A8P8;
        case EResourceFormat::B4G4R4A4_UNorm:
            return DXGI_FORMAT_B4G4R4A4_UNORM;
        case EResourceFormat::P208:
            return DXGI_FORMAT_P208;
        case EResourceFormat::V208:
            return DXGI_FORMAT_V208;
        case EResourceFormat::V408:
            return DXGI_FORMAT_V408;
        default:
            std::unreachable();
        }
    }

    EResourceFormat CastFormat(
        DXGI_FORMAT Format)
    {
        switch (Format)
        {
        case DXGI_FORMAT_UNKNOWN:
            return EResourceFormat::Unknown;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            return EResourceFormat::R32G32B32A32_Typeless;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return EResourceFormat::R32G32B32A32_Float;
        case DXGI_FORMAT_R32G32B32A32_UINT:
            return EResourceFormat::R32G32B32A32_UInt;
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return EResourceFormat::R32G32B32A32_SInt;
        case DXGI_FORMAT_R32G32B32_TYPELESS:
            return EResourceFormat::R32G32B32_Typeless;
        case DXGI_FORMAT_R32G32B32_FLOAT:
            return EResourceFormat::R32G32B32_Float;
        case DXGI_FORMAT_R32G32B32_UINT:
            return EResourceFormat::R32G32B32_UInt;
        case DXGI_FORMAT_R32G32B32_SINT:
            return EResourceFormat::R32G32B32_SInt;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            return EResourceFormat::R16G16B16A16_Typeless;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return EResourceFormat::R16G16B16A16_Float;
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            return EResourceFormat::R16G16B16A16_UNorm;
        case DXGI_FORMAT_R16G16B16A16_UINT:
            return EResourceFormat::R16G16B16A16_UInt;
        case DXGI_FORMAT_R16G16B16A16_SNORM:
            return EResourceFormat::R16G16B16A16_SNorm;
        case DXGI_FORMAT_R16G16B16A16_SINT:
            return EResourceFormat::R16G16B16A16_SInt;
        case DXGI_FORMAT_R32G32_TYPELESS:
            return EResourceFormat::R32G32_Typeless;
        case DXGI_FORMAT_R32G32_FLOAT:
            return EResourceFormat::R32G32_Float;
        case DXGI_FORMAT_R32G32_UINT:
            return EResourceFormat::R32G32_UInt;
        case DXGI_FORMAT_R32G32_SINT:
            return EResourceFormat::R32G32_SInt;
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            return EResourceFormat::R32G8X24_Typeless;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return EResourceFormat::D32_Float_S8X24_UInt;
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            return EResourceFormat::R32_Float_X8X24_Typeless;
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return EResourceFormat::X32_Typeless_G8X24_UInt;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            return EResourceFormat::R10G10B10A2_Typeless;
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            return EResourceFormat::R10G10B10A2_UNorm;
        case DXGI_FORMAT_R10G10B10A2_UINT:
            return EResourceFormat::R10G10B10A2_UInt;
        case DXGI_FORMAT_R11G11B10_FLOAT:
            return EResourceFormat::R11G11B10_Float;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            return EResourceFormat::R8G8B8A8_Typeless;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return EResourceFormat::R8G8B8A8_UNorm;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return EResourceFormat::R8G8B8A8_UNorm_SRGB;
        case DXGI_FORMAT_R8G8B8A8_UINT:
            return EResourceFormat::R8G8B8A8_UInt;
        case DXGI_FORMAT_R8G8B8A8_SNORM:
            return EResourceFormat::R8G8B8A8_SNorm;
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return EResourceFormat::R8G8B8A8_SInt;
        case DXGI_FORMAT_R16G16_TYPELESS:
            return EResourceFormat::R16G16_Typeless;
        case DXGI_FORMAT_R16G16_FLOAT:
            return EResourceFormat::R16G16_Float;
        case DXGI_FORMAT_R16G16_UNORM:
            return EResourceFormat::R16G16_UNorm;
        case DXGI_FORMAT_R16G16_UINT:
            return EResourceFormat::R16G16_UInt;
        case DXGI_FORMAT_R16G16_SNORM:
            return EResourceFormat::R16G16_SNorm;
        case DXGI_FORMAT_R16G16_SINT:
            return EResourceFormat::R16G16_SInt;
        case DXGI_FORMAT_R32_TYPELESS:
            return EResourceFormat::R32_Typeless;
        case DXGI_FORMAT_D32_FLOAT:
            return EResourceFormat::D32_Float;
        case DXGI_FORMAT_R32_FLOAT:
            return EResourceFormat::R32_Float;
        case DXGI_FORMAT_R32_UINT:
            return EResourceFormat::R32_UInt;
        case DXGI_FORMAT_R32_SINT:
            return EResourceFormat::R32_SInt;
        case DXGI_FORMAT_R24G8_TYPELESS:
            return EResourceFormat::R24G8_Typeless;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return EResourceFormat::D24_UNorm_S8_UInt;
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            return EResourceFormat::R24_UNorm_X8_Typeless;
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return EResourceFormat::X24_Typeless_G8_UInt;
        case DXGI_FORMAT_R8G8_TYPELESS:
            return EResourceFormat::R8G8_Typeless;
        case DXGI_FORMAT_R8G8_UNORM:
            return EResourceFormat::R8G8_UNorm;
        case DXGI_FORMAT_R8G8_UINT:
            return EResourceFormat::R8G8_UInt;
        case DXGI_FORMAT_R8G8_SNORM:
            return EResourceFormat::R8G8_SNorm;
        case DXGI_FORMAT_R8G8_SINT:
            return EResourceFormat::R8G8_SInt;
        case DXGI_FORMAT_R16_TYPELESS:
            return EResourceFormat::R16_Typeless;
        case DXGI_FORMAT_R16_FLOAT:
            return EResourceFormat::R16_Float;
        case DXGI_FORMAT_D16_UNORM:
            return EResourceFormat::D16_UNorm;
        case DXGI_FORMAT_R16_UNORM:
            return EResourceFormat::R16_UNorm;
        case DXGI_FORMAT_R16_UINT:
            return EResourceFormat::R16_UInt;
        case DXGI_FORMAT_R16_SNORM:
            return EResourceFormat::R16_SNorm;
        case DXGI_FORMAT_R16_SINT:
            return EResourceFormat::R16_SInt;
        case DXGI_FORMAT_R8_TYPELESS:
            return EResourceFormat::R8_Typeless;
        case DXGI_FORMAT_R8_UNORM:
            return EResourceFormat::R8_UNorm;
        case DXGI_FORMAT_R8_UINT:
            return EResourceFormat::R8_UInt;
        case DXGI_FORMAT_R8_SNORM:
            return EResourceFormat::R8_SNorm;
        case DXGI_FORMAT_R8_SINT:
            return EResourceFormat::R8_SInt;
        case DXGI_FORMAT_A8_UNORM:
            return EResourceFormat::A8_UNorm;
        case DXGI_FORMAT_R1_UNORM:
            return EResourceFormat::R1_UNorm;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            return EResourceFormat::R9G9B9E5_SharedExp;
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
            return EResourceFormat::R8G8_B8G8_UNorm;
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
            return EResourceFormat::G8R8_G8B8_UNorm;
        case DXGI_FORMAT_BC1_TYPELESS:
            return EResourceFormat::BC1_Typeless;
        case DXGI_FORMAT_BC1_UNORM:
            return EResourceFormat::BC1_UNorm;
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            return EResourceFormat::BC1_UNorm_SRGB;
        case DXGI_FORMAT_BC2_TYPELESS:
            return EResourceFormat::BC2_Typeless;
        case DXGI_FORMAT_BC2_UNORM:
            return EResourceFormat::BC2_UNorm;
        case DXGI_FORMAT_BC2_UNORM_SRGB:
            return EResourceFormat::BC2_UNorm_SRGB;
        case DXGI_FORMAT_BC3_TYPELESS:
            return EResourceFormat::BC3_Typeless;
        case DXGI_FORMAT_BC3_UNORM:
            return EResourceFormat::BC3_UNorm;
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            return EResourceFormat::BC3_UNorm_SRGB;
        case DXGI_FORMAT_BC4_TYPELESS:
            return EResourceFormat::BC4_Typeless;
        case DXGI_FORMAT_BC4_UNORM:
            return EResourceFormat::BC4_UNorm;
        case DXGI_FORMAT_BC4_SNORM:
            return EResourceFormat::BC4_SNorm;
        case DXGI_FORMAT_BC5_TYPELESS:
            return EResourceFormat::BC5_Typeless;
        case DXGI_FORMAT_BC5_UNORM:
            return EResourceFormat::BC5_UNorm;
        case DXGI_FORMAT_BC5_SNORM:
            return EResourceFormat::BC5_SNorm;
        case DXGI_FORMAT_B5G6R5_UNORM:
            return EResourceFormat::B5G6R5_UNorm;
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return EResourceFormat::B5G5R5A1_UNorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return EResourceFormat::B8G8R8A8_UNorm;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return EResourceFormat::B8G8R8X8_UNorm;
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            return EResourceFormat::R10G10B10_XR_Bias_A2_UNorm;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            return EResourceFormat::B8G8R8A8_Typeless;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return EResourceFormat::B8G8R8A8_UNorm_SRGB;
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            return EResourceFormat::B8G8R8X8_Typeless;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return EResourceFormat::B8G8R8X8_UNorm_SRGB;
        case DXGI_FORMAT_BC6H_TYPELESS:
            return EResourceFormat::BC6H_Typeless;
        case DXGI_FORMAT_BC6H_UF16:
            return EResourceFormat::BC6H_UF16;
        case DXGI_FORMAT_BC6H_SF16:
            return EResourceFormat::BC6H_SF16;
        case DXGI_FORMAT_BC7_TYPELESS:
            return EResourceFormat::BC7_Typeless;
        case DXGI_FORMAT_BC7_UNORM:
            return EResourceFormat::BC7_UNorm;
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return EResourceFormat::BC7_UNorm_SRGB;
        case DXGI_FORMAT_AYUV:
            return EResourceFormat::AYUV;
        case DXGI_FORMAT_Y410:
            return EResourceFormat::Y410;
        case DXGI_FORMAT_Y416:
            return EResourceFormat::Y416;
        case DXGI_FORMAT_NV12:
            return EResourceFormat::NV12;
        case DXGI_FORMAT_P010:
            return EResourceFormat::P010;
        case DXGI_FORMAT_P016:
            return EResourceFormat::P016;
        case DXGI_FORMAT_420_OPAQUE:
            return EResourceFormat::Opaque_420;
        case DXGI_FORMAT_YUY2:
            return EResourceFormat::YUY2;
        case DXGI_FORMAT_Y210:
            return EResourceFormat::Y210;
        case DXGI_FORMAT_Y216:
            return EResourceFormat::Y216;
        case DXGI_FORMAT_NV11:
            return EResourceFormat::NV11;
        case DXGI_FORMAT_AI44:
            return EResourceFormat::AI44;
        case DXGI_FORMAT_IA44:
            return EResourceFormat::IA44;
        case DXGI_FORMAT_P8:
            return EResourceFormat::P8;
        case DXGI_FORMAT_A8P8:
            return EResourceFormat::A8P8;
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return EResourceFormat::B4G4R4A4_UNorm;
        case DXGI_FORMAT_P208:
            return EResourceFormat::P208;
        case DXGI_FORMAT_V208:
            return EResourceFormat::V208;
        case DXGI_FORMAT_V408:
            return EResourceFormat::V408;
        default:
            std::unreachable();
        }
    }

    //

    D3D12_FILTER CastFilter(
        ESamplerFilter Filter)
    {
        switch (Filter)
        {
        case ESamplerFilter::MinMagMipPoint:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case ESamplerFilter::MinMagPoint_MipLinear:
            return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::MinPoint_MagLinear_MipPoint:
            return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::MinPoint_MagMipLinear:
            return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        case ESamplerFilter::MinLinear_MagMipPoint:
            return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case ESamplerFilter::MinLinear_MagPoint_MipLinear:
            return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::MinMagLinear_MipPoint:
            return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::MinMagMipLinear:
            return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case ESamplerFilter::Anisotropic:
            return D3D12_FILTER_ANISOTROPIC;
        case ESamplerFilter::Comparison_MinMagMipPoint:
            return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        case ESamplerFilter::Comparison_MinMagPoint_MipLinear:
            return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::Comparison_MinPoint_MagLinear_MipPoint:
            return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::Comparison_MinPoint_MagMipLinear:
            return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        case ESamplerFilter::Comparison_MinLinear_MagMipPoint:
            return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        case ESamplerFilter::Comparison_MinLinear_MagPoint_MipLinear:
            return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::Comparison_MinMagLinear_MipPoint:
            return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::Comparison_MinMagMipLinear:
            return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        case ESamplerFilter::Comparison_Anisotropic:
            return D3D12_FILTER_COMPARISON_ANISOTROPIC;
        case ESamplerFilter::Minimum_MinMagMipPoint:
            return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
        case ESamplerFilter::Minimum_MinMagPoint_MipLinear:
            return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::Minimum_MinPoint_MagLinear_MipPoint:
            return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::Minimum_MinPoint_MagMipLinear:
            return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case ESamplerFilter::Minimum_MinLinear_MagMipPoint:
            return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case ESamplerFilter::Minimum_MinLinear_MagPoint_MipLinear:
            return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::Minimum_MinMagLinear_MipPoint:
            return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::Minimum_MinMagMipLinear:
            return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
        case ESamplerFilter::Minimum_Anisotropic:
            return D3D12_FILTER_MINIMUM_ANISOTROPIC;
        case ESamplerFilter::Maximum_MinMagMipPoint:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
        case ESamplerFilter::Maximum_MinMagPoint_MipLinear:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::Maximum_MinPoint_MagLinear_MipPoint:
            return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::Maximum_MinPoint_MagMipLinear:
            return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case ESamplerFilter::Maximum_MinLinear_MagMipPoint:
            return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case ESamplerFilter::Maximum_MinLinear_MagPoint_MipLinear:
            return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case ESamplerFilter::Maximum_MinMagLinear_MipPoint:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case ESamplerFilter::Maximum_MinMagMipLinear:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
        case ESamplerFilter::Maximum_Anisotropic:
            return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
        default:
            std::unreachable();
        }
    }

    ESamplerFilter CastFilter(
        D3D12_FILTER Filter)
    {
        switch (Filter)
        {
        case D3D12_FILTER_MIN_MAG_MIP_POINT:
            return ESamplerFilter::MinMagMipPoint;
        case D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::MinMagPoint_MipLinear;
        case D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::MinPoint_MagLinear_MipPoint;
        case D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR:
            return ESamplerFilter::MinPoint_MagMipLinear;
        case D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT:
            return ESamplerFilter::MinLinear_MagMipPoint;
        case D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::MinLinear_MagPoint_MipLinear;
        case D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::MinMagLinear_MipPoint;
        case D3D12_FILTER_MIN_MAG_MIP_LINEAR:
            return ESamplerFilter::MinMagMipLinear;
        case D3D12_FILTER_ANISOTROPIC:
            return ESamplerFilter::Anisotropic;
        case D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT:
            return ESamplerFilter::Comparison_MinMagMipPoint;
        case D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::Comparison_MinMagPoint_MipLinear;
        case D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::Comparison_MinPoint_MagLinear_MipPoint;
        case D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
            return ESamplerFilter::Comparison_MinPoint_MagMipLinear;
        case D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
            return ESamplerFilter::Comparison_MinLinear_MagMipPoint;
        case D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::Comparison_MinLinear_MagPoint_MipLinear;
        case D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::Comparison_MinMagLinear_MipPoint;
        case D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:
            return ESamplerFilter::Comparison_MinMagMipLinear;
        case D3D12_FILTER_COMPARISON_ANISOTROPIC:
            return ESamplerFilter::Comparison_Anisotropic;
        case D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT:
            return ESamplerFilter::Minimum_MinMagMipPoint;
        case D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::Minimum_MinMagPoint_MipLinear;
        case D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::Minimum_MinPoint_MagLinear_MipPoint;
        case D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
            return ESamplerFilter::Minimum_MinPoint_MagMipLinear;
        case D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
            return ESamplerFilter::Minimum_MinLinear_MagMipPoint;
        case D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::Minimum_MinLinear_MagPoint_MipLinear;
        case D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::Minimum_MinMagLinear_MipPoint;
        case D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR:
            return ESamplerFilter::Minimum_MinMagMipLinear;
        case D3D12_FILTER_MINIMUM_ANISOTROPIC:
            return ESamplerFilter::Minimum_Anisotropic;
        case D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT:
            return ESamplerFilter::Maximum_MinMagMipPoint;
        case D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::Maximum_MinMagPoint_MipLinear;
        case D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::Maximum_MinPoint_MagLinear_MipPoint;
        case D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
            return ESamplerFilter::Maximum_MinPoint_MagMipLinear;
        case D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
            return ESamplerFilter::Maximum_MinLinear_MagMipPoint;
        case D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
            return ESamplerFilter::Maximum_MinLinear_MagPoint_MipLinear;
        case D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
            return ESamplerFilter::Maximum_MinMagLinear_MipPoint;
        case D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR:
            return ESamplerFilter::Maximum_MinMagMipLinear;
        case D3D12_FILTER_MAXIMUM_ANISOTROPIC:
            return ESamplerFilter::Maximum_Anisotropic;
        default:
            std::unreachable();
        }
    }

    //

    D3D12_TEXTURE_ADDRESS_MODE CastAddressMode(
        ESamplerMode AddressMode)
    {
        switch (AddressMode)
        {
        case ESamplerMode::Wrap:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case ESamplerMode::Mirror:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case ESamplerMode::Clamp:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case ESamplerMode::Border:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case ESamplerMode::MirrorOnce:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default:
            std::unreachable();
        }
    }

    ESamplerMode CastAddressMode(
        D3D12_TEXTURE_ADDRESS_MODE AddressMode)
    {
        switch (AddressMode)
        {
        case D3D12_TEXTURE_ADDRESS_MODE_WRAP:
            return ESamplerMode::Wrap;
        case D3D12_TEXTURE_ADDRESS_MODE_MIRROR:
            return ESamplerMode::Mirror;
        case D3D12_TEXTURE_ADDRESS_MODE_CLAMP:
            return ESamplerMode::Clamp;
        case D3D12_TEXTURE_ADDRESS_MODE_BORDER:
            return ESamplerMode::Border;
        case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE:
            return ESamplerMode::MirrorOnce;
        default:
            std::unreachable();
        }
    }

    //

    D3D12_COMPARISON_FUNC CastComparisonFunc(
        ECompareFunc CmpFunc)
    {
        switch (CmpFunc)
        {
        case ECompareFunc::Never:
            return D3D12_COMPARISON_FUNC_NEVER;
        case ECompareFunc::Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case ECompareFunc::Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case ECompareFunc::LessEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case ECompareFunc::Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case ECompareFunc::NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case ECompareFunc::GreaterEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case ECompareFunc::Always:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            std::unreachable();
        }
    }

    ESamplerMode CastComparisonFunc(
        D3D12_TEXTURE_ADDRESS_MODE AddressMode)
    {
        switch (AddressMode)
        {
        case D3D12_TEXTURE_ADDRESS_MODE_WRAP:
            return ESamplerMode::Wrap;
        case D3D12_TEXTURE_ADDRESS_MODE_MIRROR:
            return ESamplerMode::Mirror;
        case D3D12_TEXTURE_ADDRESS_MODE_CLAMP:
            return ESamplerMode::Clamp;
        case D3D12_TEXTURE_ADDRESS_MODE_BORDER:
            return ESamplerMode::Border;
        case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE:
            return ESamplerMode::MirrorOnce;
        default:
            std::unreachable();
        }
    }

    //

    MResourceFlags CastResourceFlags(
        D3D12_RESOURCE_FLAGS Flags)
    {
        MResourceFlags Res;
        for (auto& State : GetResourceFlagList())
        {
            if ((Flags & State.first) == State.first)
            {
                Res.Set(State.second);
            }
        }
        return Res;
    }

    D3D12_RESOURCE_FLAGS CastResourceFlags(
        const MResourceFlags& Flags)
    {
        D3D12_RESOURCE_FLAGS Res = D3D12_RESOURCE_FLAG_NONE;
        for (auto& State : GetResourceFlagList())
        {
            if (Flags.Test(State.second))
            {
                Res |= State.first;
            }
        }
        return Res;
    }

    //

    D3D_PRIMITIVE_TOPOLOGY CastPrimitiveTopology(
        PrimitiveTopology Topology)
    {

        switch (Topology)
        {
        case PrimitiveTopology::PointList:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology::LineList:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::LineStrip:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology::TriangleList:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PrimitiveTopology::LineList_Adj:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        case PrimitiveTopology::LineStrip_Adj:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        case PrimitiveTopology::TriangleList_Adj:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        case PrimitiveTopology::TriangleStrip_Adj:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        default:
        {
            if (Topology >= PrimitiveTopology::PatchList_1 &&
                Topology <= PrimitiveTopology::PatchList_32)
            {
                return D3D_PRIMITIVE_TOPOLOGY(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + uint32_t(Topology) - uint32_t(PrimitiveTopology::PatchList_1));
            }
            else
            {
                std::unreachable();
            }
        }
        }
    }
} // namespace Neon::RHI