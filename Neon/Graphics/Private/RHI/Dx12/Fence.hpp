#pragma once

#include <RHI/Fence.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12Fence : public virtual IFence
    {
    public:
        Dx12Fence() = default;

        Dx12Fence(
            uint64_t InitialValue);

        void WaitCPU(
            uint64_t Value,
            uint32_t MsWaitTime = uint32_t(-1)) override;

        void WaitGPU(
            ICommandQueue* CommandQueue,
            uint64_t       Value) override;

        uint64_t GetCompletedValue() override;

        void SignalCPU(
            uint64_t Value) override;

        void SignalGPU(
            ICommandQueue* CommandQueue,
            uint64_t       Value) override;

    private:
        Win32::ComPtr<ID3D12Fence> m_Fence;
        Win32::WinHandlePtr        m_FenceSingalEvent;
    };

    class Dx12MultiFence final : public virtual IMultiFence,
                                 public Dx12Fence
    {
        using WaitSignalList = std::list<Win32::WinHandlePtr>;

    public:
        Dx12MultiFence(
            uint64_t InitialValue);

        void WaitCPU(
            uint64_t Value,
            uint32_t MsWaitTime = uint32_t(-1)) override;

        void WaitGPU(
            ICommandQueue* CommandQueue,
            uint64_t       Value) override;

        uint64_t GetCompletedValue() override;

        void SignalCPU(
            uint64_t Value) override;

        void SignalGPU(
            ICommandQueue* CommandQueue,
            uint64_t       Value) override;

    private:
        /// <summary>
        /// Reserve new signal to be used in Wait function
        /// </summary>
        [[nodiscard]] HANDLE NewWaitSignal();

        /// <summary>
        /// free the signal in Wait function
        /// </summary>
        void FreeWaitSignal(
            HANDLE Handle);

    private:
        std::mutex m_SignalMutex;

        Win32::ComPtr<ID3D12Fence> m_Fence;
        uint64_t                   m_FenceValue = 0;
        WaitSignalList             m_ActiveSignals, m_FreeSignals;
    };
} // namespace Neon::RHI