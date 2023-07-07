#pragma once

#include <mutex>
#include <Core/Neon.hpp>

namespace Neon::Utils
{
    template<typename _Ty>
    class Singleton
    {
    public:
        Singleton() = default;
        NEON_CLASS_NO_COPYMOVE(Singleton);
        ~Singleton() = default;

        /// <summary>
        /// Construct the singleton.
        /// </summary>
        template<typename... _Args>
        static void Construct(
            _Args&&... Args)
        {
            if (auto Instance = s_Instance.load(std::memory_order_acquire))
            {
                if (std::scoped_lock Lock(s_Mutex); !s_Instance.load(std::memory_order_relaxed))
                {
                    s_Instance.store(NEON_NEW _Ty(std::forward<_Args>(Args)...), std::memory_order_release);
                }
            }
        }

        /// <summary>
        /// Destruct the singleton.
        /// </summary>
        static void Destruct()
        {
            if (auto Instance = s_Instance.exchange(nullptr, std::memory_order_acq_rel))
            {
                delete Instance;
            }
        }

        /// <summary>
        /// Get the singleton instance.
        /// </summary>
        [[nodiscard]] static _Ty* Get() noexcept
        {
            return s_Instance.load(std::memory_order_acquire);
        }

    private:
        static std::atomic<_Ty*> s_Instance;
        static std::mutex        s_Mutex;
    };

    template<typename _Ty>
    class SingletonRtti
    {
    public:
        SingletonRtti()
        {
            _Ty::Construct();
        }

        NEON_CLASS_NO_COPYMOVE(SingletonRtti);

        ~SingletonRtti()
        {
            _Ty::Destruct();
        }

        [[nodiscard]] _Ty* Get() noexcept
        {
            return _Ty::Get();
        }

        [[nodiscard]] _Ty* operator->() noexcept
        {
            return _Ty::Get();
        }
    };
} // namespace Neon::Utils