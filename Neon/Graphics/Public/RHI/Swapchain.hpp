#pragma once

#include <Math/Size2.hpp>
#include <RHI/SwapchainConfig.hpp>
#include <RHI/Commands/Common.hpp>
#include <future>

namespace Neon::Windowing
{
    class WindowApp;
} // namespace Neon::Windowing

namespace Neon::RHI
{
    class IGpuResource;
    class ICommandQueue;
    class IFence;
    class ICopyCommandList;

    class ISwapchain
    {
    public:
        ISwapchain() = default;
        NEON_CLASS_NO_COPYMOVE(ISwapchain);
        virtual ~ISwapchain() = default;

        /// <summary>
        /// Get the swapchain instance.
        /// </summary>
        [[nodiscard]] static ISwapchain* Get();

        /// <summary>
        /// Prepare frame for rendering.
        /// </summary>
        virtual void PrepareFrame() = 0;

        /// <summary>
        /// Render frame and present it to the swapchain.
        /// </summary>
        virtual void Present(
            float FrameTime) = 0;

        /// <summary>
        /// Get the swapchain's window.
        /// </summary>
        [[nodiscard]] virtual Windowing::WindowApp* GetWindow() = 0;

        /// <summary>
        /// Get the swapchain's window size.
        /// </summary>
        [[nodiscard]] virtual const Size2I& GetSize() = 0;

        /// <summary>
        /// Get the swapchain's format (backbuffer format).
        /// </summary>
        [[nodiscard]] virtual EResourceFormat GetFormat() = 0;

        /// <summary>
        /// Get the swapchain's backbuffer.
        /// </summary>
        [[nodiscard]] virtual IGpuResource* GetBackBuffer() = 0;

        /// <summary>
        /// Get the swapchain's backbuffer view.
        /// </summary>
        [[nodiscard]] virtual CpuDescriptorHandle GetBackBufferView() = 0;

        /// <summary>
        /// Set swapchain's size.
        /// </summary>
        virtual void Resize(
            const Size2I&   Size,
            EResourceFormat NewFormat = EResourceFormat::Unknown) = 0;

        /// <summary>
        /// Get the swapchain's backbuffer count.
        /// </summary>
        [[nodiscard]] virtual uint32_t GetBackbufferCount() = 0;

        /// <summary>
        /// Get the swapchain's queue.
        /// </summary>
        [[nodiscard]] virtual ICommandQueue* GetQueue(
            CommandQueueType Type) = 0;

        /// <summary>
        /// Enqueue a copy command list to be executed.
        /// </summary>
        template<typename _FnTy, typename... _Args>
        uint64_t RequestCopy(
            _FnTy&& Task,
            _Args&&... Args)
        {
            return EnqueueRequestCopy(std::bind_back(
                std::forward<_FnTy>(Task),
                std::forward<_Args>(Args)...));
        }

        /// <summary>
        /// Wait for a copy command list to be executed.
        /// </summary>
        virtual void WaitForCopy(
            ICommandQueue* Queue,
            uint64_t       FenceValue) = 0;

    protected:
        /// <summary>
        /// Enqueue a copy command list to be executed.
        /// </summary>
        virtual uint64_t EnqueueRequestCopy(
            std::function<void(ICopyCommandList*)> Task) = 0;

    public:
        /// <summary>
        /// Check if VSync is enabled.
        /// </summary>
        [[nodiscard]] bool IsVSyncEnabled() const noexcept
        {
            return m_IsVSyncEnabled;
        }

        /// <summary>
        /// Set VSync enabled.
        /// </summary>
        void SetVSyncEnabled(
            bool IsEnabled) noexcept
        {
            m_IsVSyncEnabled = IsEnabled;
        }

    protected:
        bool m_IsVSyncEnabled : 1 = false;
    };
} // namespace Neon::RHI