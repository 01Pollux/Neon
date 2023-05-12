#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>

namespace Neon::RHI
{
    [[nodiscard]] static constexpr auto GetTranslationList() noexcept
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

    D3D12_RESOURCE_STATES CastResourceStates(
        EResourceState States)
    {
        for (auto& State : GetTranslationList())
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
        for (auto& State : GetTranslationList())
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
        for (auto& State : GetTranslationList())
        {
            if ((States & State.first) == State.first)
            {
                Res.Set(State.second);
            }
        }
        return Res;
    }
} // namespace Neon::RHI