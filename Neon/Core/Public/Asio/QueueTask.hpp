#pragma once

#include <future>
#include <queue>
#include <Asio/NullMutex.hpp>

namespace Neon::Asio
{
    template<typename _MtxType>
    class QueueTaskT
    {
        using TaskType = std::move_only_function<void()>;

    public:
        /// <summary>
        /// Push a task to the queue and return a future
        /// </summary>
        template<std::invocable _FnTy>
        auto Push(
            _FnTy&& Function)
        {
            using _RetTy          = std::invoke_result_t<_FnTy>;
            using _PackagedTaskTy = std::packaged_task<_RetTy()>;

            _PackagedTaskTy Task{ std::forward<_FnTy>(Function) };
            std::future     Future = Task.get_future();

            if constexpr (std::is_same_v<_MtxType, NullMutex>)
            {
                m_Tasks.emplace([Task = std::move(Task)]() mutable
                                { Task(); });
            }
            else
            {
                std::scoped_lock Lock{ m_Mutex };
                m_Tasks.emplace([Task = std::move(Task)]() mutable
                                { Task(); });
            }

            return Future;
        }

        /// <summary>
        /// Pop the first task from the queue and execute it
        /// </summary>
        void Pop()
        {
            TaskType Task;
            if constexpr (std::is_same_v<_MtxType, NullMutex>)
            {
                Task = std::move(m_Tasks.front());
                m_Tasks.pop();
            }
            else
            {
                std::scoped_lock Lock{ m_Mutex };
                Task = std::move(m_Tasks.front());
                m_Tasks.pop();
            }
            Task();
        }

        /// <summary>
        /// Check if the queue is empty
        /// </summary>
        [[nodiscard]] bool Empty() const
        {
            if constexpr (std::is_same_v<_MtxType, NullMutex>)
            {
                return m_Tasks.empty();
            }
            else
            {
                std::scoped_lock Lock{ m_Mutex };
                return m_Tasks.empty();
            }
        }

    private:
        _MtxType             m_Mutex;
        std::queue<TaskType> m_Tasks;
    };

    using QueueTaskST = QueueTaskT<NullMutex>;
    using QueueTaskMT = QueueTaskT<std::mutex>;
} // namespace Neon::Asio