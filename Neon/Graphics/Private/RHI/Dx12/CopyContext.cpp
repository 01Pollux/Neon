#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Budget.hpp>
#include <Private/RHI/Dx12/Device.hpp>

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

    CopyContextManager::CopyContextManager(
        ISwapchain* Swapchain) :
        m_CopyQueue(CreateCopyQueue())
    {
        for (size_t i = 0; i < m_Threads.size(); i++)
        {
            m_Threads[i] = std::jthread(
                [this, Swapchain](std::stop_token Token, size_t Index)
                {
                    auto& Context = m_CommandContexts[Index];

                    Dx12CopyCommandList CopyCommandList(Swapchain);
                    CopyCommandList.AttachCommandList(Context.CommandList.Get());

                    while (!Token.stop_requested())
                    {
                        std::unique_lock Lock(m_QueueMutex);
                        if (!m_TaskWaiter.wait(Lock, Token, [this]
                                               { return !m_Queue.empty(); }))
                        {
                            break;
                        }

                        ThrowIfFailed(Context.CommandAllocator->Reset());
                        ThrowIfFailed(Context.CommandList->Reset(Context.CommandAllocator.Get(), nullptr));

#if NEON_DEBUG
                        try
                        {
                            for (size_t i = 0; i < CommandsToHandleCount && !m_Queue.empty(); i++)
                            {
                                m_Queue.front()(&CopyCommandList);
                                m_Queue.pop();
                            }
                        }
                        catch (...)
                        {
                            std::terminate();
                        }
#else
                        for (size_t i = 0; i < CommandsToHandleCount && !m_Queue.empty(); i++)
                        {
                            m_Queue.front()(&CopyCommandList);
                            m_Queue.pop();
                        }
#endif
                        ID3D12CommandList* CommandLists[]{ Context.CommandList.Get() };

                        ThrowIfFailed(Context.CommandList->Close());
                        m_CopyQueue->ExecuteCommandLists(1, CommandLists);

                        Lock.unlock();
                    }
                },
                i);
        }
    }

    std::future<void> CopyContextManager::EnqueueCopy(
        std::function<void(ICopyCommandList*)> Task)
    {
        PackagedTaskType PackagedTask(Task);
        auto             Future = PackagedTask.get_future();

        {
            std::scoped_lock Lock(m_QueueMutex);
            m_Queue.emplace(std::move(PackagedTask));
        }

        m_TaskWaiter.notify_one();
        return Future;
    }

    void CopyContextManager::Shutdown()
    {
        m_TaskWaiter.notify_all();
        m_Threads = {};
    }

    Win32::ComPtr<ID3D12CommandQueue> CopyContextManager::CreateCopyQueue()
    {
        Win32::ComPtr<ID3D12CommandQueue> Result;

        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_COMMAND_QUEUE_DESC Desc{
            .Type = D3D12_COMMAND_LIST_TYPE_COPY
        };
        ThrowIfFailed(Dx12Device->CreateCommandQueue(
            &Desc,
            IID_PPV_ARGS(&Result)));

        return Result;
    }
} // namespace Neon::RHI