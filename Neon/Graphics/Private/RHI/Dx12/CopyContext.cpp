#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/FrameManager.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Log/Logger.hpp>
#include <chrono>

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

        RenameObject(CommandAllocator.Get(), STR("Main Copy Command Allocator"));
        RenameObject(CommandList.Get(), STR("Main Copy Command List"));
    }

    template<typename _Ty, typename... _Args>
    void ProcessTasks(
        _Ty& TasksQueue,
        _Args&&... Args)
    {
#ifndef NEON_DIST
        for (size_t i = 0; i < CopyContextManager::CommandsToHandleCount && !TasksQueue.empty(); i++)
        {
            auto [Task, Promise] = std::move(TasksQueue.back());
            TasksQueue.pop_back();
            try
            {
                Task(std::forward<_Args>(Args)...);
                Promise.set_value();
            }
            catch (const std::exception& Exception)
            {
                NEON_FATAL("Exception in copy context thread: {}", Exception.what());
            }
        }

#else
        for (size_t i = 0; i < CopyContextManager::CommandsToHandleCount && !TasksQueue.empty(); i++)
        {
            auto [Task, Promise] = std::move(TasksQueue.back());
            TasksQueue.pop_back();

            Task(std::forward<_Args>(Args)...);
            Promise.set_value();
        }
#endif
    };

    CopyContextManager::CopyContextManager() :
        m_CopyQueue(CommandQueueType::Copy),
        m_CopyFence(0)
    {
        RenameObject(m_CopyQueue.Get(), STR("Main Copy Command Queue'"));

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
                                               { return !m_CopyTasks.empty(); }))
                        {
                            break;
                        }

                        ThrowIfFailed(Context.CommandAllocator->Reset());
                        ThrowIfFailed(Context.CommandList->Reset(Context.CommandAllocator.Get(), nullptr));

                        ProcessTasks(m_CopyTasks, &CommandList);

                        ID3D12CommandList* CommandLists[]{ Context.CommandList.Get() };

                        ThrowIfFailed(Context.CommandList->Close());
                        m_CopyQueue.Get()->ExecuteCommandLists(1, CommandLists);
                        m_CopyFence.SignalGPU(&m_CopyQueue, m_CopyId);

                        auto MainQueue = Dx12Swapchain::Get()->GetQueue(true);
                        m_CopyFence.WaitGPU(MainQueue, m_CopyId);
                        ProcessTasks(m_PostCopyTasks);

                        auto OldValue = m_CopyId++;
                        Lock.unlock();
                        m_CopyFence.WaitCPU(OldValue);
                    }
                },
                i);
        }
    }

    std::future<void> CopyContextManager::EnqueueCopy(
        std::move_only_function<void(ICommandList*)> CopyTask,
        std::move_only_function<void()>              PostCopyTask)
    {
        NEON_ASSERT(CopyTask);
        FutureType Future;

        {
            std::scoped_lock Lock(m_QueueMutex);
            m_CopyTasks.emplace_back(PackagedCopyTaskType(std::move(CopyTask)));
            Future = m_PostCopyTasks.emplace_back(PackagedPostCopyTaskType(std::move(PostCopyTask))).Promise.get_future();
        }

        m_TaskWaiter.notify_one();
        return Future;
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