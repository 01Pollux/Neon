#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Fence.hpp>

#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>

namespace Neon::RHI
{
    IFence* IFence::Create(
        uint64_t InitialValue)
    {
        return NEON_NEW Dx12Fence(InitialValue);
    }

    IMultiFence* IMultiFence::Create(
        uint64_t InitialValue)
    {
        return NEON_NEW Dx12MultiFence(InitialValue);
    }

    //

    Dx12Fence::Dx12Fence(
        uint64_t InitialValue) :
        m_FenceSingalEvent(CreateEvent(nullptr, 0, 0, nullptr))
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateFence(
            InitialValue,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_Fence)));

        RenameObject(m_Fence, STR("RHI::Fence"));
    }

    void Dx12Fence::WaitCPU(
        uint64_t Value,
        uint32_t MsWaitTime)
    {
        m_Fence->SetEventOnCompletion(Value, m_FenceSingalEvent.Get());
        WaitForSingleObject(m_FenceSingalEvent.Get(), MsWaitTime);
    }

    void Dx12Fence::WaitGPU(
        ICommandQueue* CommandQueue,
        uint64_t       Value)
    {
        auto DxCommandQueue = static_cast<Dx12CommandQueue*>(CommandQueue)->Get();
        DxCommandQueue->Wait(m_Fence.Get(), Value);
    }

    uint64_t Dx12Fence::GetCompletedValue()
    {
        return m_Fence->GetCompletedValue();
    }

    void Dx12Fence::SignalCPU(
        uint64_t Value)
    {
        m_Fence->Signal(Value);
    }

    void Dx12Fence::SignalGPU(
        ICommandQueue* CommandQueue,
        uint64_t       Value)
    {
        auto DxCommandQueue = static_cast<Dx12CommandQueue*>(CommandQueue)->Get();
        DxCommandQueue->Signal(m_Fence.Get(), Value);
    }

    //

    Dx12MultiFence::Dx12MultiFence(
        uint64_t InitialValue)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateFence(
            InitialValue,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_Fence)));

        RenameObject(m_Fence, STR("RHI::Fence"));
    }

    void Dx12MultiFence::WaitCPU(
        uint64_t Value,
        uint32_t MsWaitTime)
    {
        HANDLE WaitSignal = NewWaitSignal();
        m_Fence->SetEventOnCompletion(Value, WaitSignal);
        WaitForSingleObject(WaitSignal, MsWaitTime);
        FreeWaitSignal(WaitSignal);
    }

    void Dx12MultiFence::WaitGPU(
        ICommandQueue* CommandQueue,
        uint64_t       Value)
    {
        std::lock_guard SignalLock(m_SignalMutex);

        auto DxCommandQueue = static_cast<Dx12CommandQueue*>(CommandQueue)->Get();
        DxCommandQueue->Wait(m_Fence.Get(), Value);
    }

    uint64_t Dx12MultiFence::GetCompletedValue()
    {
        std::lock_guard SignalLock(m_SignalMutex);
        return m_Fence->GetCompletedValue();
    }

    void Dx12MultiFence::SignalCPU(
        uint64_t Value)
    {
        std::lock_guard SignalLock(m_SignalMutex);
        m_Fence->Signal(Value);
    }

    void Dx12MultiFence::SignalGPU(
        ICommandQueue* CommandQueue,
        uint64_t       Value)
    {
        std::lock_guard SignalLock(m_SignalMutex);

        auto DxCommandQueue = static_cast<Dx12CommandQueue*>(CommandQueue)->Get();
        DxCommandQueue->Signal(m_Fence.Get(), Value);
    }

    //

    HANDLE Dx12MultiFence::NewWaitSignal()
    {
        std::lock_guard SignalLock(m_SignalMutex);

        HANDLE Handle;
        if (!m_FreeSignals.empty())
        {
            Handle = m_ActiveSignals.emplace_back(std::move(m_FreeSignals.back())).Get();
            m_FreeSignals.pop_back();
        }
        else
        {
            Handle = m_ActiveSignals.emplace_back(CreateEvent(nullptr, 0, 0, nullptr)).Get();
        }

        return Handle;
    }

    void Dx12MultiFence::FreeWaitSignal(
        HANDLE Handle)
    {
        std::lock_guard SignalLock(m_SignalMutex);

        auto Iter = std::ranges::find_if(
            m_ActiveSignals, [Handle](const auto& CurHandle)
            { return CurHandle.Get() == Handle; });
        m_FreeSignals.emplace_back(std::move(*Iter));
        m_ActiveSignals.erase(Iter);
    }
} // namespace Neon::RHI