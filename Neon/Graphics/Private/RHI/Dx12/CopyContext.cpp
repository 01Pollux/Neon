#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/FrameManager.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    CopyContextManager::CommandContext::CommandContext()
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        ThrowIfFailed(Dx12Device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_COPY,
            IID_PPV_ARGS(&CommandAllocator)));

        ThrowIfFailed(Dx12Device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_COPY,
            CommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&CommandList)));

        ThrowIfFailed(CommandList->Close());
    }

    CopyContextManager::CopyContextManager() :
        m_CopyQueue(CommandQueueType::Copy),
        m_CopyFence(0)
    {
        for (size_t i = 0; i < m_Threads.size(); i++)
        {
            m_Threads[i] = std::jthread(
                [this](std::stop_token Token, size_t Index)
                {
                    auto& Context = m_CommandContexts[Index];

                    Dx12CommandList CommandList;
                    CommandList.AttachCommandList(Context.CommandList.Get());

                    while (!Token.stop_requested())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));

                        std::unique_lock Lock(m_QueueMutex);
                        if (!m_TaskWaiter.wait(Lock, Token, [this]
                                               { return !m_Queue.empty(); }))
                        {
                            break;
                        }

                        ThrowIfFailed(Context.CommandAllocator->Reset());
                        ThrowIfFailed(Context.CommandList->Reset(Context.CommandAllocator.Get(), nullptr));

#ifndef NEON_DIST
                        try
                        {
                            for (size_t i = 0; i < CommandsToHandleCount && !m_Queue.empty(); i++)
                            {
                                auto& [Task, Promise] = m_Queue.front();
                                Task(&CommandList);
                                m_Queue.pop();
                            }
                        }
                        catch (const std::exception& Exception)
                        {
                            NEON_ERROR("Exception in copy context thread: {}", Exception.what());
                            std::terminate();
                        }
#else
                        for (size_t i = 0; i < CommandsToHandleCount && !m_Queue.empty(); i++)
                        {
                            auto& [Task, Promise] = m_Queue.front();
                            Task(&CopyCommandList);
                            m_Queue.pop();
                        }
#endif
                        ID3D12CommandList* CommandLists[]{ Context.CommandList.Get() };

                        ThrowIfFailed(Context.CommandList->Close());
                        m_CopyQueue.Get()->ExecuteCommandLists(1, CommandLists);
                        m_CopyFence.SignalGPU(&m_CopyQueue, m_CopyId);
                        m_CopyFence.WaitCPU(m_CopyId);

                        m_CopyId++;
                    }
                },
                i);
        }
    }

    uint64_t CopyContextManager::EnqueueCopy(
        std::function<void(ICommandList*)> Task)
    {
        uint64_t CopyId;
        {
            std::scoped_lock Lock(m_QueueMutex);
            CopyId = m_CopyId;
            m_Queue.emplace(PackagedTaskType(Task));
        }

        m_TaskWaiter.notify_one();
        return CopyId;
    }

    void CopyContextManager::Shutdown()
    {
        m_TaskWaiter.notify_all();
        m_Threads = {};
    }

    Dx12CommandQueue* CopyContextManager::GetQueue() noexcept
    {
        return &m_CopyQueue;
    }

    Dx12Fence* CopyContextManager::GetQueueFence() noexcept
    {
        return &m_CopyFence;
    }
} // namespace Neon::RHI