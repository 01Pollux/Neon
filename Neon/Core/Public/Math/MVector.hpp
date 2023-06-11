#pragma once

#include <array>
#include <numbers>

#define MVECTOR_IMPL_MATH_OP(VecType, Symbol)                                   \
    VecType&              operator##Symbol##=(const VecType& Other) noexcept;   \
    VecType&              operator##Symbol##=(value_type Val) noexcept;         \
    [[nodiscard]] VecType operator##Symbol(const VecType& Other) const noexcept \
    {                                                                           \
        VecType Tmp   = *this;                                                  \
        Tmp##Symbol## = Other;                                                  \
        return Tmp;                                                             \
    }                                                                           \
    [[nodiscard]] VecType operator##Symbol(value_type Val) const noexcept       \
    {                                                                           \
        VecType Tmp   = *this;                                                  \
        Tmp##Symbol## = Val;                                                    \
        return Tmp;                                                             \
    }

#define MVECTOR_IMPL_ACCESSOR(Index, Name)     \
    constexpr value_type Name() const noexcept \
    {                                          \
        return (*this)[Index];                 \
    }                                          \
                                               \
    constexpr value_type& Name() noexcept      \
    {                                          \
        return (*this)[Index];                 \
    }

#define MVECTOR_IMPL_MATHVEC_DECLARATIONS(VecType)                                                                  \
                                                                                                                    \
    [[nodiscard]] bool IsNan() const noexcept;                                                                      \
                                                                                                                    \
    void Negate() noexcept;                                                                                         \
                                                                                                                    \
    [[nodiscard]] value_type Dot(const VecType& Other) const noexcept;                                              \
                                                                                                                    \
    [[nodiscard]] value_type LengthSqr() const noexcept;                                                            \
                                                                                                                    \
    [[nodiscard]] value_type Length() const noexcept;                                                               \
                                                                                                                    \
    [[nodiscard]] bool IsGreaterThan(const VecType& Other) const noexcept;                                          \
                                                                                                                    \
    [[nodiscard]] bool IsLessThan(const VecType& Other) const noexcept;                                             \
                                                                                                                    \
    void Normalize() noexcept;                                                                                      \
                                                                                                                    \
    [[nodiscard]] bool NearEqual(const VecType& Other,                                                              \
                                 value_type     Tolerance = std::numeric_limits<value_type>::epsilon()) const noexcept; \
                                                                                                                    \
    [[nodiscard]] bool Within(const VecType& Min, const VecType& Max) const noexcept;                               \
                                                                                                                    \
    /* Multiply, add, and assign to this (ie: *this = *this * A + B ). */                                           \
    void MultAdd(const VecType& A, const VecType& B) noexcept;                                                      \
                                                                                                                    \
    VecType& operator=(value_type Val) noexcept;                                                                    \
                                                                                                                    \
    [[nodiscard]] VecType operator-() const noexcept;                                                               \
                                                                                                                    \
    [[nodiscard]] VecType Min(const VecType& Other) const noexcept;                                                 \
                                                                                                                    \
    [[nodiscard]] VecType Max(const VecType& Other) const noexcept;                                                 \
                                                                                                                    \
    [[nodiscard]] value_type DistanceTo(const VecType& Other) const noexcept                                        \
    {                                                                                                               \
        return (Other - *this).Length();                                                                            \
    }                                                                                                               \
                                                                                                                    \
    [[nodiscard]] value_type DistanceToSqr(const VecType& Other) const noexcept                                     \
    {                                                                                                               \
        return (Other - *this).LengthSqr();                                                                         \
    }                                                                                                               \
                                                                                                                    \
    MVECTOR_IMPL_MATH_OP(VecType, +);                                                                               \
    MVECTOR_IMPL_MATH_OP(VecType, -);                                                                               \
    MVECTOR_IMPL_MATH_OP(VecType, /);                                                                               \
    MVECTOR_IMPL_MATH_OP(VecType, *)

namespace Neon
{
    template<typename _Ty, size_t _Size>
    class MVector
    {
    public:
        using value_type      = _Ty;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using pointer         = _Ty*;
        using const_pointer   = const _Ty*;

        using array_type      = std::array<value_type, _Size>;
        using difference_type = array_type::difference_type;

        using iterator       = array_type::iterator;
        using const_iterator = array_type::const_iterator;

        using reverse_iterator       = array_type::reverse_iterator;
        using const_reverse_iterator = array_type::const_reverse_iterator;

        constexpr MVector() = default;

        constexpr MVector(std::initializer_list<value_type> list) noexcept
        {
            std::copy_n(list.begin(), std::min(list.size(), size()), begin());
        }

        constexpr MVector(const array_type& arr) noexcept :
            m_Data{ arr }
        {
        }

        template<typename _OTy, size_t _OSize>
            requires std::negation_v<std::is_same<MVector, MVector<_OTy, _OSize>>>
        constexpr MVector(const MVector<_OTy, _OSize>& Other) noexcept
        {
            if constexpr (std::is_same_v<value_type, _OTy>)
            {
                std::copy_n(Other.begin(), std::min(Other.size(), size()), begin());
            }
            else
            {
                auto meit = begin();
                for (auto it = Other.begin(), end = it + std::min(Other.size(), size()); it != end; ++it, ++meit)
                    *meit = static_cast<value_type>(*it);
            }
        }

        [[nodiscard]] constexpr const_reference operator[](difference_type i) const noexcept
        {
            return m_Data[i];
        }
        [[nodiscard]] constexpr reference operator[](difference_type i) noexcept
        {
            return m_Data[i];
        }

        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
            return m_Data.data();
        }

        [[nodiscard]] pointer data() noexcept
        {
            return m_Data.data();
        }

        [[nodiscard]] constexpr const array_type& Get() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]] array_type& Get() noexcept
        {
            return m_Data;
        }

        [[nodiscard]] constexpr size_t size() const noexcept
        {
            return _Size;
        }

        template<typename _OTy>
            requires(sizeof(_OTy) == sizeof(_Ty) * size())
        [[nodiscard]] constexpr const _OTy& To() const noexcept
        {
            return *std::bit_cast<const _OTy*>(data());
        }

        template<typename _OTy>
            requires(sizeof(_OTy) == sizeof(_Ty) * size())
        [[nodiscard]] _OTy& To() noexcept
        {
            return *std::bit_cast<_OTy*>(data());
        }

        constexpr auto operator<=>(const MVector&) const = default;

        void UnsafeCopyFrom(const_pointer pval)
        {
            for (reference v : m_Data)
            {
                v = *pval;
                ++pval;
            }
        }

        void UnsafeCopyTo(pointer pval) const
        {
            for (const_reference v : m_Data)
            {
                *pval = v;
                ++pval;
            }
        }

    public:
        constexpr void fill(const_reference val)
        {
            m_Data.fill(val);
        }

        constexpr void swap(array_type& other) noexcept(std::is_nothrow_swappable_v<_Ty>)
        {
            m_Data.swap(other);
        }

        [[nodiscard]] constexpr iterator begin() noexcept
        {
            return m_Data.begin();
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept
        {
            return m_Data.begin();
        }

        [[nodiscard]] constexpr iterator end() noexcept
        {
            return m_Data.end();
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept
        {
            return m_Data.end();
        }

        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
        {
            return m_Data.rbegin();
        }

        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
        {
            return m_Data.rbegin();
        }

        [[nodiscard]] constexpr reverse_iterator rend() noexcept
        {
            return m_Data.rend();
        }

        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
        {
            return m_Data.rend();
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept
        {
            return begin();
        }

        [[nodiscard]] constexpr const_iterator cend() const noexcept
        {
            return end();
        }

        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
        {
            return rbegin();
        }

        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
        {
            return rend();
        }

    private:
        array_type m_Data{};
    };

    template<typename _Ty, typename... _Rest>
    MVector(_Ty, _Rest...) -> MVector<typename std::_Enforce_same<_Ty, _Rest...>::type, 1 + sizeof...(_Rest)>;

    template<typename _Ty>
    [[nodiscard]] constexpr _Ty DegreesToRadians(_Ty Deg) noexcept
    {
        return Deg * (static_cast<float>(std::numbers::pi) / 180.f);
    }

    template<typename _Ty>
    [[nodiscard]] constexpr _Ty RadiansToDegrees(_Ty Rad) noexcept
    {
        return Rad * (180.f / static_cast<float>(std::numbers::pi));
    }
} // namespace Neon
