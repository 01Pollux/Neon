#pragma once

#include <Core/Neon.hpp>
#include <bitset>

namespace boost::serialization
{
    class access;
} // namespace boost::serialization

namespace Neon
{
    template<typename _ETy>
    class Bitmask
    {
    public:
        static constexpr size_t bits_count = static_cast<size_t>(_ETy::_Last_Enum);

        using type             = _ETy;
        using bitset_type      = std::bitset<bits_count>;
        using bitset_reference = bitset_type::reference;

    public:
        static constexpr Bitmask FromEnum(
            _ETy Val) noexcept
        {
            Bitmask mask;
            mask.Set(Val);
            return mask;
        }

        constexpr Bitmask() = default;

        constexpr Bitmask(
            const Bitmask& Other) :
            m_Bitset(Other.m_Bitset)
        {
        }

        constexpr Bitmask(
            uint64_t Val) noexcept :
            m_Bitset(Val)
        {
        }

        constexpr Bitmask(
            uint32_t Val) noexcept :
            m_Bitset(static_cast<uint64_t>(Val))
        {
        }

        template<class _Ty, class _Tr, class _Alloc>
        explicit Bitmask(
            const std::basic_string<_Ty, _Tr, _Alloc>&              Str,
            typename std::basic_string<_Ty, _Tr, _Alloc>::size_type Pos   = 0,
            typename std::basic_string<_Ty, _Tr, _Alloc>::size_type Count = std::basic_string<_Ty, _Tr, _Alloc>::npos,
            _Ty Zero = static_cast<_Ty>('0'), _Ty one = static_cast<_Ty>('1')) :
            m_Bitset(Str, Pos, Count, Zero, one)
        {
        }

        template<class _Ty>
        explicit Bitmask(
            const _Ty*                                 Ntcts,
            typename std::basic_string<_Ty>::size_type Count = std::basic_string<_Ty>::npos,
            _Ty                                        Zero  = static_cast<_Ty>('0'),
            _Ty                                        One   = static_cast<_Ty>('1')) :
            m_Bitset(Ntcts, Count, Zero, One)
        {
        }

        Bitmask& Set() noexcept
        {
            m_Bitset.set();
            return *this;
        }
        Bitmask& Set(
            type Pos,
            bool Val = true)
        {
            m_Bitset.set(static_cast<size_t>(Pos), Val);
            return *this;
        }
        [[nodiscard]] Bitmask& Flip() noexcept
        {
            m_Bitset.flip();
            return *this;
        }
        [[nodiscard]] Bitmask& Flip(
            type Pos)
        {
            m_Bitset.flip(static_cast<size_t>(Pos));
            return *this;
        }
        [[nodiscard]] Bitmask& Reset() noexcept
        {
            m_Bitset.reset();
            return *this;
        }
        [[nodiscard]] Bitmask& Reset(
            type Pos)
        {
            m_Bitset.reset(static_cast<size_t>(Pos));
            return *this;
        }

        [[nodiscard]] bool Test(
            type Pos) const
        {
            return m_Bitset.test(static_cast<size_t>(Pos));
        }
        [[nodiscard]] bool Any() const noexcept
        {
            return m_Bitset.any();
        }
        [[nodiscard]] bool None() const noexcept
        {
            return m_Bitset.none();
        }
        [[nodiscard]] bool All() const noexcept
        {
            return m_Bitset.all();
        }

        [[nodiscard]] size_t Size() const noexcept
        {
            return m_Bitset.size();
        }
        [[nodiscard]] size_t Count() const noexcept
        {
            return m_Bitset.count();
        }

        [[nodiscard]] bool operator[](
            type Pos) const
        {
            return m_Bitset.operator[](static_cast<size_t>(Pos));
        }
        [[nodiscard]] auto operator[](
            type Pos)
        {
            return m_Bitset.operator[](static_cast<size_t>(Pos));
        }
        [[nodiscard]] bool operator[](
            size_t Pos) const
        {
            return m_Bitset.operator[](Pos);
        }
        [[nodiscard]] auto operator[](
            size_t Pos)
        {
            return m_Bitset.operator[](Pos);
        }

        template<class _Ty = char, class _Tr = std::char_traits<_Ty>, class _Alloc = std::allocator<_Ty>>
        [[nodiscard]] auto ToString(
            _Ty Zero = static_cast<_Ty>('0'),
            _Ty One  = static_cast<_Ty>('1'))
            const
        {
            return m_Bitset.to_string(Zero, One);
        }

        [[nodiscard]] uint64_t ToUllong() const
        {
            return m_Bitset.to_ullong();
        }
        [[nodiscard]] uint32_t ToUlong() const
        {
            return m_Bitset.to_ulong();
        }

        [[nodiscard]] bitset_type& Get() noexcept
        {
            return m_Bitset;
        }
        [[nodiscard]] operator bitset_type&() noexcept
        {
            return m_Bitset;
        }
        [[nodiscard]] const bitset_type& Get() const noexcept
        {
            return m_Bitset;
        }
        [[nodiscard]] operator const bitset_type&() const noexcept
        {
            return m_Bitset;
        }

        [[nodiscard]] auto operator~() const noexcept
        {
            return ebitset(m_Bitset.operator~());
        }
        [[nodiscard]] auto operator<<(type Pos) const noexcept
        {
            return ebitset(m_Bitset.operator<<(static_cast<size_t>(Pos)));
        }
        [[nodiscard]] auto operator>>(type Pos) const noexcept
        {
            return ebitset(m_Bitset.operator>>(static_cast<size_t>(Pos)));
        }
        [[nodiscard]] bool operator==(type Pos) const noexcept
        {
            return m_Bitset.operator==(static_cast<size_t>(Pos));
        }

        auto operator>>=(
            type Pos) noexcept
        {
            m_Bitset.operator>>=(static_cast<size_t>(Pos));
            return *this;
        }
        auto operator<<=(
            type Pos) noexcept
        {
            m_Bitset.operator<<=(static_cast<size_t>(Pos));
            return *this;
        }
        auto operator^=(
            const bitset_type& Right) noexcept
        {
            m_Bitset.operator^=(Right);
            return *this;
        }
        auto operator^=(
            const Bitmask& Right) noexcept
        {
            operator^=(Right.Get());
            return *this;
        }
        auto operator|=(
            const bitset_type& Right) noexcept
        {
            m_Bitset.operator|=(Right);
            return *this;
        }
        auto operator|=(
            const Bitmask& Right) noexcept
        {
            operator|=(Right.Get());
            return *this;
        }
        auto operator&=(
            const bitset_type& Right) noexcept
        {
            m_Bitset.operator&=(Right);
            return *this;
        }
        auto operator&=(
            const Bitmask& Right) noexcept
        {
            operator&=(Right.Get());
            return *this;
        }

        [[nodiscard]] bool TestAll(
            const Bitmask& Other) const
        {
            uint64_t Flags = Other.ToUllong();
            return (Flags & ToUllong()) == Flags;
        }

        [[nodiscard]] bool TestAny(
            const Bitmask& Other) const
        {
            uint64_t Flags = Other.ToUllong();
            return (Flags & ToUllong()) != 0;
        }

        [[nodiscard]] bool TestNone(
            const Bitmask& Other) const
        {
            return !TestAny(Other);
        }

        static constexpr Bitmask Empty() noexcept
        {
            return {};
        }

    private:
        friend class boost::serialization::access;
        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t  Version);

    private:
        bitset_type m_Bitset;
    };

    //

    template<typename _ETy>
    _NODISCARD Bitmask<_ETy> operator&(
        const Bitmask<_ETy>& Lhs,
        const Bitmask<_ETy>& Rhs) noexcept
    {
        auto ret = Lhs;
        return ret &= Rhs;
    }

    template<typename _ETy>
    _NODISCARD Bitmask<_ETy> operator|(
        const Bitmask<_ETy>& Lhs,
        const Bitmask<_ETy>& Rhs) noexcept
    {
        auto ret = Lhs;
        return ret |= Rhs;
    }

    template<typename _ETy>
    _NODISCARD Bitmask<_ETy> operator^(
        const Bitmask<_ETy>& Lhs,
        const Bitmask<_ETy>& Rhs) noexcept
    {
        auto ret = Lhs;
        return ret ^= Rhs;
    }

    //

    template<typename... _ETy>
    [[nodiscard]] constexpr auto BitMask_Or(
        _ETy... Vals) noexcept
    {
        using FirstType = std::tuple_element_t<0, std::tuple<_ETy...>>;
        uint64_t Mask   = ((1Ui64 << static_cast<uint64_t>(Vals)) | ...);
        return Bitmask<FirstType>(Mask);
    }

    template<typename... _ETy>
    [[nodiscard]] constexpr auto BitMask_Xor(
        _ETy... Vals) noexcept
    {
        using FirstType = std::tuple_element_t<0, std::tuple<_ETy...>>;
        uint64_t Mask   = ((1Ui64 << static_cast<uint64_t>(Vals)) ^ ...);
        return Bitmask<FirstType>(Mask);
    }

    template<typename _ETy>
    [[nodiscard]] constexpr auto BitMask_Not(
        _ETy Vals) noexcept
    {
        return Bitmask<_ETy>{ (~(1Ui64 << static_cast<uint64_t>(Vals))) };
    }

    template<typename... _ETy>
    [[nodiscard]] constexpr auto BitMask_And(
        _ETy... Vals) noexcept
    {
        using FirstType = std::tuple_element_t<0, std::tuple<_ETy...>>;
        uint64_t Mask   = ((1Ui64 << static_cast<uint64_t>(Vals)) & ...);
        return Bitmask<FirstType>(Mask);
    }
} // namespace Neon
