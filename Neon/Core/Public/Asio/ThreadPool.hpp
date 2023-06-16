#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <future>
#include <condition_variable>
#include <functional>

namespace Neon::Asio
{
    template<std::invocable _FnTy = std::function<void()>>
    class ThreadPool
    {
    public:
        using FunctionReturnType = std::invoke_result_t<_FnTy>;
        using PackagedTaskType   = std::packaged_task<FunctionReturnType()>;
        using FutureType         = std::future<FunctionReturnType>;

        ThreadPool(
            size_t ThreadsCount = std::thread::hardware_concurrency())
        {
            Resize(ThreadsCount);
        }

        /// <summary>
        /// Resizes the thread pool.
        /// </summary>
        void Resize(
            size_t ThreadsCount)
        {
            size_t OldSize = m_Threads.size();
            m_Threads.resize(ThreadsCount);

            for (size_t i = OldSize; i < m_Threads.size(); i++)
            {
                m_Threads[i] = std::jthread(
                    [this](std::stop_token Stop)
                    {
                        while (!Stop.stop_requested())
                        {
                            std::unique_lock Lock(m_QueueMutex);
                            if (!m_TaskWaiter.wait(Lock, Stop, [this]
                                                   { return !m_Queue.empty(); }))
                            {
                                break;
                            }

                            auto Task = std::move(m_Queue.front());
                            m_Queue.pop();

                            Lock.unlock();

#if NEON_DEBUG
                            try
                            {
                                std::invoke(Task);
                            }
                            catch (...)
                            {
                                std::terminate();
                            }
#else
                            std::invoke(Task);
#endif
                        }
                    });
            }
        }

        /// <summary>
        /// Enqueues a task.
        /// </summary>
        template<std::invocable _Fn, typename... _Args>
        FutureType Enqueue(
            _Fn&& Function,
            _Args&&... Args)
        {
            using ReturnType = std::invoke_result_t<_Fn, _Args...>;

            PackagedTaskType Task(std::bind(std::forward<_Fn>(Function), std::forward<_Args>(Args)...));
            auto             Future = Task.get_future();

            {
                std::scoped_lock Lock(m_QueueMutex);
                m_Queue.emplace(std::move(Task));
            }

            m_TaskWaiter.notify_one();
            return Future;
        }

        /// <summary>
        /// Returns the number of threads in the pool.
        /// </summary>
        [[nodiscard]] size_t GetThreadsCount() const noexcept
        {
            return m_Threads.size();
        }

    private:
        std::vector<std::jthread>    m_Threads;
        std::mutex                   m_QueueMutex;
        std::queue<PackagedTaskType> m_Queue;
        std::condition_variable_any  m_TaskWaiter;
    };
} // namespace Neon::Asio