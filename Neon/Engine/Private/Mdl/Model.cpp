#include <EnginePCH.hpp>
#include <Mdl/Model.hpp>
#include <RHI/Commands/Context.hpp>

namespace Neon::Mdl
{
    std::pair<void*, RHI::UBufferPoolHandle> Model::PeekVertexBuffer() const noexcept
    {
        if (m_VertexBuffer.index() == 0)
        {
            auto& Buffer = std::get<RHI::USyncGpuResource>(m_VertexBuffer).Get();
            return PeekVertexBuffer(0, Buffer->GetSize());
        }
        else
        {
            // We dont need to know the size, since we will just taking pointer to the buffer
            return PeekVertexBuffer(0, 0);
        }
    }

    std::pair<void*, RHI::UBufferPoolHandle> Model::PeekVertexBuffer(
        size_t Offset,
        size_t Size) const noexcept
    {
        std::pair<void*, RHI::UBufferPoolHandle> Data;

        const RHI::UBufferPoolHandle* FinalHandle = nullptr;
        if (m_VertexBuffer.index() == 0)
        {
            auto& Buffer = std::get<RHI::USyncGpuResource>(m_VertexBuffer).Get();
            Data.second  = RHI::UBufferPoolHandle(
                Size,
                Buffer->GetDesc().Alignment,
                RHI::IGlobalBufferPool::BufferType::ReadOnly);
            FinalHandle = &Data.second;

            RHI::CommandContext CommandContext;
            auto                CommandList = CommandContext.Append();
            CommandList->CopyBufferRegion(FinalHandle->AsReadBack(), FinalHandle->Offset, Buffer.get(), Offset, Size);
        }
        else
        {
            FinalHandle = &std::get<RHI::UBufferPoolHandle>(m_VertexBuffer);
        }

        Data.first = FinalHandle->AsUpload().Map() + FinalHandle->Offset + Offset;
        return Data;
    }

    void Model::UnmapVertexBuffer() const noexcept
    {
        if (m_VertexBuffer.index() != 0)
        {
            auto& Buffer = std::get<RHI::UBufferPoolHandle>(m_VertexBuffer);
            Buffer.AsUpload().Unmap();
        }
    }

    //

    std::pair<void*, RHI::UBufferPoolHandle> Model::PeekIndexBuffer() const noexcept
    {
        if (m_VertexBuffer.index() == 0)
        {
            auto& Buffer = std::get<RHI::USyncGpuResource>(m_VertexBuffer).Get();
            return PeekIndexBuffer(0, Buffer->GetSize());
        }
        else
        {
            // We dont need to know the size, since we will just taking pointer to the buffer
            return PeekIndexBuffer(0, 0);
        }
    }

    std::pair<void*, RHI::UBufferPoolHandle> Model::PeekIndexBuffer(
        size_t Offset,
        size_t Size) const noexcept
    {
        std::pair<void*, RHI::UBufferPoolHandle> Data;

        const RHI::UBufferPoolHandle* FinalHandle = nullptr;
        if (m_VertexBuffer.index() == 0)
        {
            auto& Buffer = std::get<RHI::USyncGpuResource>(m_VertexBuffer).Get();
            Data.second  = RHI::UBufferPoolHandle(
                Size,
                Buffer->GetDesc().Alignment,
                RHI::IGlobalBufferPool::BufferType::ReadOnly);
            FinalHandle = &Data.second;

            RHI::CommandContext CommandContext;
            auto                CommandList = CommandContext.Append();
            CommandList->CopyBufferRegion(FinalHandle->AsReadBack(), FinalHandle->Offset, Buffer.get(), Offset, Size);
        }
        else
        {
            FinalHandle = &std::get<RHI::UBufferPoolHandle>(m_VertexBuffer);
        }

        Data.first = FinalHandle->AsUpload().Map() + FinalHandle->Offset + Offset;
        return Data;
    }

    void Model::UnmapIndexBuffer() const noexcept
    {
        if (m_IndexBuffer.index() != 0)
        {
            auto& Buffer = std::get<RHI::UBufferPoolHandle>(m_IndexBuffer);
            Buffer.AsUpload().Unmap();
        }
    }
} // namespace Neon::Mdl