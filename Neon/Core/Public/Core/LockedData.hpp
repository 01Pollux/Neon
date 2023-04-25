#pragma once

#include <mutex>
#include <memory>

namespace Neon
{
    template<typename _Ty, typename _MtxTy = std::mutex>
    class LockableData
    {
        friend class LockedData;

    public:
        class LockedData;

        [[nodiscard]] LockedData Lock()
        {
            return LockedData(this);
        }

        [[nodiscard]] _Ty& GetUnsafe() noexcept
        {
            return m_Data;
        }

        [[nodiscard]] const _Ty& GetUnsafe() const noexcept
        {
            return m_Data;
        }

    private:
        _MtxTy m_Lock;
        _Ty    m_Data;
    };

    template<typename _Ty, typename _MtxTy>
    class [[nodiscard]] LockableData<_Ty, _MtxTy>::LockedData
    {
        using LockableDataDeleter = decltype([](LockableData* Data)
                                             { Data->m_Lock.unlock(); });

    public:
        explicit LockedData(LockableData* Data) :
            m_DataPointer(Data)
        {
            m_DataPointer->m_Lock.lock();
        }

        [[nodiscard]] auto operator->() const noexcept
        {
            return std::addressof(m_DataPointer->m_Data);
        }

        [[nodiscard]] auto& Get() const noexcept
        {
            return m_DataPointer->m_Data;
        }

        [[nodiscard]] auto& operator*() const noexcept
        {
            return m_DataPointer->m_Data;
        }

    private:
        std::unique_ptr<LockableData, LockableDataDeleter> m_DataPointer;
    };
} // namespace Neon