#pragma once

namespace Neon::Asio
{
    class NullMutex
    {
    public:
        void lock() noexcept
        {
        }

        void unlock() noexcept
        {
        }

        bool try_lock() noexcept
        {
            return true;
        }
    };
} // namespace Neon::Asio