project "NeonCoroutines"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()

    includedirs
    {
        "%{CommonDir.Neon.Coroutines}"
    }

    files 
    {
        "%{CommonDir.Neon.Coroutines}/cppcoro/awaitable_traits.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/is_awaitable.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/async_auto_reset_event.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/async_manual_reset_event.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/async_generator.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/async_mutex.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/async_latch.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/async_scope.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/broken_promise.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/cancellation_registration.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/cancellation_source.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/cancellation_token.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/task.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/sequence_barrier.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/sequence_traits.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/single_producer_sequencer.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/multi_producer_sequencer.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/shared_task.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/shared_task.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/single_consumer_event.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/single_consumer_async_auto_reset_event.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/sync_wait.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/task.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/io_service.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/config.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/on_scope_exit.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/file_share_mode.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/file_open_mode.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/file_buffering_mode.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/file.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/fmap.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/when_all.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/when_all_ready.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/resume_on.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/schedule_on.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/generator.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/readable_file.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/recursive_generator.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/writable_file.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/read_only_file.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/write_only_file.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/read_write_file.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/file_read_operation.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/file_write_operation.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/static_thread_pool.hpp",

        "%{CommonDir.Neon.Coroutines}/cppcoro/net/ip_address.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/net/ip_endpoint.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/net/ipv4_address.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/net/ipv4_endpoint.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/net/ipv6_address.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/net/ipv6_endpoint.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket.hpp",

        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/void_value.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/when_all_ready_awaitable.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/when_all_counter.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/when_all_task.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/get_awaiter.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/is_awaiter.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/any.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/sync_wait_task.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/unwrap_reference.hpp",
        "%{CommonDir.Neon.Coroutines}/cppcoro/detail/lightweight_manual_reset_event.hpp",

        "%{prj.location}/Private/cppcoro/cancellation_state.hpp",
        "%{prj.location}/Private/cppcoro/socket_helpers.hpp",
        "%{prj.location}/Private/cppcoro/auto_reset_event.hpp",
        "%{prj.location}/Private/cppcoro/spin_wait.hpp",
        "%{prj.location}/Private/cppcoro/spin_mutex.hpp",

        "%{prj.location}/Private/cppcoro/async_auto_reset_event.cpp",
        "%{prj.location}/Private/cppcoro/async_manual_reset_event.cpp",
        "%{prj.location}/Private/cppcoro/async_mutex.cpp",
        "%{prj.location}/Private/cppcoro/cancellation_state.cpp",
        "%{prj.location}/Private/cppcoro/cancellation_token.cpp",
        "%{prj.location}/Private/cppcoro/cancellation_source.cpp",
        "%{prj.location}/Private/cppcoro/cancellation_registration.cpp",
        "%{prj.location}/Private/cppcoro/lightweight_manual_reset_event.cpp",
        "%{prj.location}/Private/cppcoro/ip_address.cpp",
        "%{prj.location}/Private/cppcoro/ip_endpoint.cpp",
        "%{prj.location}/Private/cppcoro/ipv4_address.cpp",
        "%{prj.location}/Private/cppcoro/ipv4_endpoint.cpp",
        "%{prj.location}/Private/cppcoro/ipv6_address.cpp",
        "%{prj.location}/Private/cppcoro/ipv6_endpoint.cpp",
        "%{prj.location}/Private/cppcoro/static_thread_pool.cpp",
        "%{prj.location}/Private/cppcoro/auto_reset_event.cpp",
        "%{prj.location}/Private/cppcoro/spin_wait.cpp",
        "%{prj.location}/Private/cppcoro/spin_mutex.cpp"
    }

    filter { "system:windows" }
        links
        {
            "Synchronization.lib"
        }
        defines "_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING"
        files
        {
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket.hpp",
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket_accept_operation.hpp",
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket_connect_operation.hpp",
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket_disconnect_operation.hpp",
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket_recv_operation.hpp",
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket_recv_from_operation.hpp",
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket_send_operation.hpp",
            "%{CommonDir.Neon.Coroutines}/cppcoro/net/socket_send_to_operation.hpp",

            "%{prj.location}/Private/cppcoro/win32.cpp",
            "%{prj.location}/Private/cppcoro/io_service.cpp",
            "%{prj.location}/Private/cppcoro/file.cpp",
            "%{prj.location}/Private/cppcoro/readable_file.cpp",
            "%{prj.location}/Private/cppcoro/writable_file.cpp",
            "%{prj.location}/Private/cppcoro/read_only_file.cpp",
            "%{prj.location}/Private/cppcoro/write_only_file.cpp",
            "%{prj.location}/Private/cppcoro/read_write_file.cpp",
            "%{prj.location}/Private/cppcoro/file_read_operation.cpp",
            "%{prj.location}/Private/cppcoro/file_write_operation.cpp",
            "%{prj.location}/Private/cppcoro/socket_helpers.cpp",
            "%{prj.location}/Private/cppcoro/socket.cpp",
            "%{prj.location}/Private/cppcoro/socket_accept_operation.cpp",
            "%{prj.location}/Private/cppcoro/socket_connect_operation.cpp",
            "%{prj.location}/Private/cppcoro/socket_disconnect_operation.cpp",
            "%{prj.location}/Private/cppcoro/socket_send_operation.cpp",
            "%{prj.location}/Private/cppcoro/socket_send_to_operation.cpp",
            "%{prj.location}/Private/cppcoro/socket_recv_operation.cpp",
            "%{prj.location}/Private/cppcoro/socket_recv_from_operation.cpp"
        }
    filter {}

    filter { "system:linux" }
        files
        {
            "%{CommonDir.Neon.Coroutines}/cppcoro/detail/linux.hpp",
            
            "%{prj.location}/Private/cppcoro/linux.cpp",
            "%{prj.location}/Private/cppcoro/io_service.cpp"
        }
    filter {}