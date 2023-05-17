#pragma once

#include <list>

namespace Neon::Allocator
{
    template<typename _Ty>
    class FreeList
    {
        using ListType = std::list<_Ty>;

    public:
        using Iterator      = typename ListType::iterator;
        using ConstIterator = typename ListType::const_iterator;

        /// <summary>
        /// Allocate from free list with option to reconstruct
        /// </summary>
        template<bool _InitializeExisting = false, typename... _Args>
        [[nodiscard]] Iterator Allocate(
            _Args&&... Args)
        {
            if (m_FreeObjects.empty())
            {
                m_ActiveObjects.emplace_back(std::forward<_Args>(Args)...);
                return std::prev(m_ActiveObjects.end());
            }
            else
            {
                m_ActiveObjects.splice(m_ActiveObjects.end(), m_FreeObjects, m_FreeObjects.begin());
                auto It = std::prev(m_ActiveObjects.end());
                if constexpr (_InitializeExisting)
                {
                    std::construct_at(&*It, std::forward<_Args>(Args)...);
                }
                return It;
            }
        }

        /// <summary>
        /// Allocate and reconstruct object
        /// </summary>
        template<typename... _Args>
        [[nodiscard]] Iterator EmplaceBack(
            _Args&&... Args)
        {
            return Allocate<true>(std::forward<_Args>(Args)...);
        }

        /// <summary>
        /// Release object with option to destruct
        /// </summary>
        template<bool _Destruct = false>
        void Free(
            ConstIterator Object)
        {
            if constexpr (_Destruct)
            {
                std::destroy_at(&*Object);
            }
            m_FreeObjects.splice(m_FreeObjects.end(), m_ActiveObjects, Object);
        }

        /// <summary>
        /// Release and destruct object
        /// </summary>
        void Release(
            ConstIterator Object)
        {
            Free<true>(Object);
        }

        /// <summary>
        /// Get list of active objects
        /// </summary>
        [[nodiscard]] auto& GetActives() const noexcept
        {
            return m_ActiveObjects;
        }

        /// <summary>
        /// Get list of free objects
        /// </summary>
        [[nodiscard]] auto& GetFree() const noexcept
        {
            return m_FreeObjects;
        }

        /// <summary>
        /// Clear list of active and free objects
        /// </summary>
        void Clear()
        {
            m_ActiveObjects.clear();
            m_FreeObjects.clear();
        }

        /// <summary>
        /// Clear list of active only
        /// </summary>
        void ClearActives()
        {
            m_FreeObjects.splice(m_FreeObjects.end(), m_ActiveObjects);
        }

        /// <summary>
        /// Emplace object to free list
        /// </summary>
        void Reserve(
            _Ty&& Entries)
        {
            m_FreeObjects.emplace_back(std::move(Entries));
        }

    private:
        ListType m_FreeObjects, m_ActiveObjects;
    };
} // namespace Neon::Allocator