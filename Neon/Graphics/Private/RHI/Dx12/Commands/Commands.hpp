#pragma once

#include <RHI/Commands/Commands.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    constexpr CommandQueueType CastCommandQueueType(
        D3D12_COMMAND_LIST_TYPE Type)
    {
        switch (Type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            return CommandQueueType::Graphics;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            return CommandQueueType::Compute;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            return CommandQueueType::Copy;
        default:
            std::unreachable();
        }
    }

    constexpr D3D12_COMMAND_LIST_TYPE CastCommandQueueType(
        CommandQueueType Type)
    {
        switch (Type)
        {
        case CommandQueueType::Graphics:
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case CommandQueueType::Compute:
            return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case CommandQueueType::Copy:
            return D3D12_COMMAND_LIST_TYPE_COPY;
        default:
            std::unreachable();
        }
    }
} // namespace Neon::RHI