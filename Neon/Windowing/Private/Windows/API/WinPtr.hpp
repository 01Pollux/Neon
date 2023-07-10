#pragma once

#include <Core/Neon.hpp>
#include <wrl/client.h>
#include <memory>

namespace Neon::WinAPI
{
    using Microsoft::WRL::ComPtr;

    template<typename HandleType, HandleType InvalidHandleValue, bool _IsFile = false>
    class WinHandlePtrImpl
    {
    public:
        explicit WinHandlePtrImpl(
            HandleType Handle = InvalidHandleValue) :
            m_Handle(Handle)
        {
        }

        NEON_CLASS_NO_COPY(WinHandlePtrImpl);

        WinHandlePtrImpl(WinHandlePtrImpl&& Handle) noexcept :
            m_Handle(std::exchange(Handle.m_Handle, InvalidHandleValue))
        {
        }

        WinHandlePtrImpl& operator=(WinHandlePtrImpl&& Handle) noexcept
        {
            if (this != &Handle)
            {
                m_Handle = std::exchange(Handle.m_Handle, InvalidHandleValue);
            }
            return *this;
        }

        ~WinHandlePtrImpl()
        {
            Release();
        }

    public:
        operator bool() const noexcept
        {
            if constexpr (_IsFile)
            {
                return m_Handle != INVALID_HANDLE_VALUE;
            }
            else
            {
                return m_Handle != InvalidHandleValue;
            }
        }

        [[nodiscard]] HandleType Get() const noexcept
        {
            return m_Handle;
        }

        void Release() noexcept
        {
            if (*this)
            {
                CloseHandle(m_Handle);
            }
        }

    private:
        HandleType m_Handle = InvalidHandleValue;
    };

    using WinHandlePtr     = WinHandlePtrImpl<HANDLE, nullptr>;
    using WinFileHandlePtr = WinHandlePtrImpl<HANDLE, nullptr, true>;
} // namespace Neon::WinAPI
