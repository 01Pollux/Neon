#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>

using namespace Neon;

class RuntimeSample : public Runtime::DefaultGameEngine
{
public:
    void Initialize(
        const Config::EngineConfig& Config) override
    {
        DefaultGameEngine::Initialize(Config);
        RegisterInterface<Runtime::IEngineRuntime, Runtime::EngineWorldRuntime>();
    }
};

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false },
    };
    return RunEngine<RuntimeSample>(Config);
}
//
// #include <chrono>
// #include <cppcoro/async_mutex.hpp>
// #include <cppcoro/schedule_on.hpp>
// #include <cppcoro/static_thread_pool.hpp>
// #include <cppcoro/sync_wait.hpp>
// #include <cppcoro/task.hpp>
// #include <cppcoro/when_all.hpp>
// #include <iostream>
// #include <string>
// #include <thread>
// #include <vector>
//
// inline constexpr auto WORK_ITEMS = 5;
//
// unsigned int get_tid()
//{
//    auto p = std::this_thread::get_id();
//    return *reinterpret_cast<unsigned int*>(&p);
//}
//
// cppcoro::task<> fill_number(int i, std::vector<int>& ints,
//                            cppcoro::async_mutex& mutex)
//{
//    std::cout << std::format("Thread {}: producing {}\n", get_tid(), i);
//    std::this_thread::sleep_for(
//        std::chrono::milliseconds((WORK_ITEMS - i) * 200));
//
//    {
//        auto lock = co_await mutex.scoped_lock_async(); // remember to co_await!
//        ints.emplace_back(i);
//    }
//
//    std::cout << std::format("Thread {}: produced {}\n", get_tid(), i);
//    co_return;
//}
//
// cppcoro::task<std::vector<int>> do_routine_work(
//    cppcoro::static_thread_pool& thread_pool)
//{
//    auto mutex = cppcoro::async_mutex{};
//    auto ints  = std::vector<int>{};
//    ints.reserve(WORK_ITEMS);
//
//    std::cout << std::format("Thread {}: starting routine work\n",
//                             get_tid());
//
//    co_await thread_pool.schedule();
//
//    std::cout << std::format("Thread {}: running first pooled job\n",
//                             get_tid());
//
//    std::vector<cppcoro::task<>> tasks;
//    for (int i = 0; i < WORK_ITEMS; ++i)
//    {
//        tasks.emplace_back(
//            cppcoro::schedule_on(thread_pool, fill_number(i, ints, mutex)));
//    }
//
//    co_await cppcoro::when_all_ready(std::move(tasks));
//
//    co_return ints;
//}
//
// int main()
//{
//    auto thread_pool = cppcoro::static_thread_pool{ 3 };
//
//    std::cout << std::format("Thread {}: starting\n", get_tid());
//    auto work = do_routine_work(thread_pool);
//
//    std::cout << std::format("Thread {}: starting work\n",
//                             get_tid());
//    const auto ints = cppcoro::sync_wait(work);
//
//    std::cout << std::format("Thread {}: work done. Produced ints are: ",
//                             get_tid());
//    for (auto i : ints)
//    {
//        std::cout << i << ", ";
//    }
//
//    std::cout << std::endl;
//    return 0;
//}
