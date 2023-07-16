#pragma once

#include <cppcoro/task.hpp>
#include <cppcoro/shared_task.hpp>
#include <cppcoro/generator.hpp>
#include <cppcoro/async_generator.hpp>

namespace Neon::Asio
{
    template<typename _Return = void>
    using CoLazy = cppcoro::task<_Return>;

    template<typename _Return = void>
    using CoSharedLazy = cppcoro::shared_task<_Return>;

    template<typename _Yield = void>
    using CoGenerator = cppcoro::generator<_Yield>;

    template<typename _Yield = void>
    using CoAsyncGenerator = cppcoro::async_generator<_Yield>;
} // namespace Neon::Asio