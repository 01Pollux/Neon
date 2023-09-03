#pragma once

#include <array>
#include <Core/String.hpp>

namespace Neon
{
    struct VersionType
    {
    public:
        enum class Type : unsigned char
        {
            Major,
            Minor,
            Build,
            Revision
        };
        using ValueType = unsigned short;

        constexpr VersionType() = default;

        constexpr VersionType(
            ValueType Maj,
            ValueType Min,
            ValueType Build = 0,
            ValueType Rev   = 0) noexcept :
            m_Major(Maj),
            m_Minor(Min),
            m_Build(Build),
            m_Revision(Rev)
        {
        }

        constexpr VersionType(
            const std::array<ValueType, 4>& Arr) noexcept :
            m_Major(Arr[0]),
            m_Minor(Arr[1]),
            m_Build(Arr[2]),
            m_Revision(Arr[3])
        {
        }

        template<StringType _StrTy>
        constexpr VersionType(
            const _StrTy& Str) noexcept :
            VersionType(FromString(Str))
        {
        }

        [[nodiscard]] constexpr ValueType operator[](
            Type Ty) const noexcept
        {
            switch (Ty)
            {
            case Type::Major:
                return m_Major;
            case Type::Minor:
                return m_Minor;
            case Type::Build:
                return m_Build;
            case Type::Revision:
            default:
                return m_Revision;
            }
        }

        [[nodiscard]] constexpr ValueType& operator[](
            Type Ty) noexcept
        {
            switch (Ty)
            {
            case Type::Major:
                return m_Major;
            case Type::Minor:
                return m_Minor;
            case Type::Build:
                return m_Build;
            case Type::Revision:
            default:
                return m_Revision;
            }
        }

        [[nodiscard]] constexpr ValueType& Major() noexcept
        {
            return m_Major;
        }
        [[nodiscard]] constexpr ValueType Major() const noexcept
        {
            return m_Major;
        }

        [[nodiscard]] constexpr ValueType& Minor() noexcept
        {
            return m_Minor;
        }
        [[nodiscard]] constexpr ValueType Minor() const noexcept
        {
            return m_Minor;
        }

        [[nodiscard]] constexpr ValueType& Build() noexcept
        {
            return m_Build;
        }
        [[nodiscard]] constexpr ValueType Build() const noexcept
        {
            return m_Build;
        }

        [[nodiscard]] constexpr ValueType& Revision() noexcept
        {
            return m_Revision;
        }
        [[nodiscard]] constexpr ValueType Revision() const noexcept
        {
            return m_Revision;
        }

        /// <summary>
        /// Convert to array
        /// </summary>
        [[nodiscard]] constexpr auto ToArray() const noexcept
        {
            return std::array{ Major(), Minor(), Build(), Revision() };
        }

        /// <summary>
        /// Convert to array
        /// </summary>
        [[nodiscard]] constexpr auto ToString() const noexcept
        {
            return StringUtils::Format(
                "{:d}.{:d}.{:d}.{:d}",
                Major(),
                Minor(),
                Build(),
                Revision());
        }

        /// <summary>
        /// Convert from string to version
        /// </summary>
        template<StringType _StrTy>
        [[nodiscard]] static constexpr VersionType FromString(
            const _StrTy& Str)
        {
            int         Iter   = 0;
            Type        CurVer = Type::Major;
            VersionType Ver;

            for (const auto C : Str)
            {
                if (CurVer > Type::Revision)
                    break;

                if (C == '.')
                {
                    Iter   = 0;
                    CurVer = Type(int(CurVer) + 1);
                    continue;
                }
                else if (C < '0' || C > '9')
                {
                    break;
                }

                Iter *= 10;
                Iter += C - '0';

                Ver[CurVer] = Iter;
            }

            return Ver;
        }

        constexpr auto operator<=>(
            const VersionType&) const noexcept = default;

    private:
        ValueType
            m_Major    = 0,
            m_Minor    = 0,
            m_Build    = 0,
            m_Revision = 0;
    };
} // namespace Neon