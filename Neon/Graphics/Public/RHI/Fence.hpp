#pragma once

#include <Math/Size2.hpp>

namespace Neon::RHI
{
    class ICommandQueue;

    class IFence
    {
    public:
        virtual ~IFence() = default;

        [[nodiscard]] static IFence* Create(
            uint64_t InitialValue = 0);

        /// Stall cpu until the fence is signaled
        /// </summary>
        virtual void WaitCPU(
            uint64_t Value,
            uint32_t MsWaitTime = uint32_t(-1)) = 0;

        /// <summary>
        /// Stall gpu until the fence is signaled
        /// </summary>
        virtual void WaitGPU(
            ICommandQueue* CommandQueue,
            uint64_t       Value) = 0;

        /// <summary>
        /// Get last completed value by the fence
        /// </summary>
        [[nodiscard]] virtual uint64_t GetCompletedValue() = 0;

        /// <summary>
        /// Signals the fence cpu side
        /// </summary>
        virtual void SignalCPU(
            uint64_t Value) = 0;

        /// <summary>
        /// Signals the fence gpu side
        /// </summary>
        virtual void SignalGPU(
            ICommandQueue* CommandQueue,
            uint64_t       Value) = 0;

    public:
        /// <summary>
        /// Stall cpu until the fence is signaled
        /// </summary>
        void CmpWaitCPU(
            uint64_t Value,
            uint32_t MsWaitTime = uint32_t(-1))
        {
            if (GetCompletedValue() < Value)
            {
                WaitCPU(Value, MsWaitTime);
            }
        }

        /// <summary>
        /// Stall gpu until the fence is signaled
        /// </summary>
        void CmpWaitGPU(
            ICommandQueue* CommandQueue,
            uint64_t       Value)
        {
            if (GetCompletedValue() < Value)
            {
                WaitGPU(CommandQueue, Value);
            }
        }
    };

    class IMultiFence : public virtual IFence
    {
    public:
        [[nodiscard]] static IMultiFence* Create(
            uint64_t InitialValue = 0);
    };
} // namespace Neon::RHI