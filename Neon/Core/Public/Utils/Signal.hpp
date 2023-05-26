#pragma once

#include <list>
#include <functional>
#include <boost/pool/pool.hpp>

namespace Neon::Utils
{
    template<typename... _Args>
    class Signal
    {
    public:
        using DelegateType = std::function<void(_Args...)>;

        using CallbackDelegateData = std::pair<DelegateType, uint64_t>;
        using CallbackDelegateList = std::list<CallbackDelegateData /*, boost::fast_pool_allocator<CallbackDelegateData>*/>;

    public:
        /// <summary>
        /// Add a listener to the delegate list
        /// </summary>
        [[nodiscard]] uint64_t Listen(
            std::function<void(_Args...)>&& Listener)
        {
            return m_Listeners.emplace_back(std::move(Listener), m_NextHandle++).second;
        }

        /// <summary>
        /// Remove a listener from the delegate list
        /// </summary>
        void Drop(
            uint64_t Id)
        {
            // Remove from the list the listener with the given id
            std::erase_if(
                m_Listeners,
                [Id](const auto& Listener)
                { return Listener.second == Id; });
        }

        /// <summary>
        /// Invoke the listeners with the given arguments
        /// </summary>
        void Broadcast(
            _Args... Args)
        {
            for (const auto& Listener : m_Listeners)
            {
                Listener.first(std::forward<_Args>(Args)...);
            }
        }

    private:
        CallbackDelegateList m_Listeners;
        uint64_t             m_NextHandle{ 0 };
    };

    template<typename... _Args>
    class SignalHandle
    {
    public:
        SignalHandle() = default;
        SignalHandle(
            Signal<_Args...>&                         Sig,
            typename Signal<_Args...>::DelegateType&& Delegate) :
            m_Signal(&Sig),
            m_Id(m_Signal->Listen(std::move(Delegate)))
        {
        }

        SignalHandle(
            const SignalHandle& Other) = delete;
        SignalHandle(
            SignalHandle&& Other) noexcept :
            m_Signal(std::exchange(Other.m_Signal, nullptr)),
            m_Id(std::exchange(Other.m_Id, uint64_t(~0)))
        {
        }

        SignalHandle& operator=(
            const SignalHandle& Other) = delete;
        SignalHandle& operator=(
            SignalHandle&& Other) noexcept
        {
            if (this != &Other)
            {
                m_Signal = std::exchange(Other.m_Signal, nullptr);
                m_Id     = std::exchange(Other.m_Id, uint64_t(~0));
            }
            return *this;
        }

        ~SignalHandle()
        {
            if (m_Signal)
            {
                m_Signal->Drop(m_Id);
                m_Signal = nullptr;
            }
        }

        /// <summary>
        /// Attach the listener to the signal delegate list
        /// </summary>
        void Attach(
            Signal<_Args...>&                         Sig,
            typename Signal<_Args...>::DelegateType&& Delegate)
        {
            Drop();
            m_Signal = &Sig;
            m_Id     = m_Signal->Listen(std::move(Delegate));
        }

        /// <summary>
        /// Release the ownership of the handle
        /// </summary>
        void Release()
        {
            m_Signal = nullptr;
        }

        /// <summary>
        /// Drop the listener from the signal delegate list
        /// </summary>
        void Drop()
        {
            if (m_Signal)
            {
                m_Signal->Drop(m_Id);
                m_Id     = uint64_t(~0);
                m_Signal = nullptr;
            }
        }

    private:
        Utils::Signal<_Args...>* m_Signal = nullptr;
        uint64_t                 m_Id     = uint64_t(~0);
    };

    //

#define NEON_SIGNAL_DECL(Name, ...)                              \
    namespace Neon::Signals                                      \
    {                                                            \
        using S##Name  = Neon::Utils::Signal<__VA_ARGS__>;       \
        using SH##Name = Neon::Utils::SignalHandle<__VA_ARGS__>; \
    }

#define NEON_SIGNAL_INST(Name)          \
public:                                 \
    [[nodiscard]] auto& Name() noexcept \
    {                                   \
        return m_##Name;                \
    }                                   \
                                        \
private:                                \
    Neon::Signals::S##Name m_##Name

} // namespace Neon::Utils