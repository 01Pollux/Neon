#pragma once

#include <istream>
#include <boost/archive/basic_archive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/item_version_type.hpp>

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include <boost/archive/basic_binary_oprimitive.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/archive/detail/common_oarchive.hpp>
#include <boost/archive/detail/register_archive.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

namespace Neon::IO
{
    /////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
    // InArchive.hpp

    // (C) Copyright 2002-7 Robert Ramey - http://www.rrsd.com .
    // Use, modification and distribution is subject to the Boost Software
    // License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    // http://www.boost.org/LICENSE_1_0.txt)

    //  See http://www.boost.org for updates, documentation, and revision history.

    /////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
    // exception to be thrown if integer read from archive doesn't fit
    // variable being loaded
    class InArchive_exception : public boost::archive::archive_exception
    {
    public:
        enum exception_code
        {
            incompatible_integer_size
        } m_exception_code;
        InArchive_exception(exception_code c = incompatible_integer_size) :
            boost::archive::archive_exception(boost::archive::archive_exception::other_exception),
            m_exception_code(c)
        {
        }
        virtual const char* what() const throw()
        {
            const char* msg = "programmer error";
            switch (m_exception_code)
            {
            case incompatible_integer_size:
                msg = "integer cannot be represented";
                break;
            default:
                msg = boost::archive::archive_exception::what();
                assert(false);
                break;
            }
            return msg;
        }
    };

    /////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
    // "Portable" input binary archive.  It addresses integer size and endianness so
    // that binary archives can be passed across systems. Note:floating point types
    // not addressed here
    class InArchive : public boost::archive::basic_binary_iprimitive<
                          InArchive,
                          std::istream::char_type,
                          std::istream::traits_type>,
                      public boost::archive::detail::common_iarchive<
                          InArchive>
    {
        typedef boost::archive::basic_binary_iprimitive<
            InArchive,
            std::istream::char_type,
            std::istream::traits_type>
            primitive_base_t;
        typedef boost::archive::detail::common_iarchive<
            InArchive>
            archive_base_t;
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
    public:
#else
        friend archive_base_t;
        friend primitive_base_t; // since with override load below
        friend class boost::archive::detail::interface_iarchive<
            InArchive>;
        friend class boost::archive::load_access;

    protected:
#endif
        unsigned int m_flags;

        template<size_t _Size>
        void load_impl(
            std::array<uint8_t, _Size>& bytes)
        {
            uint8_t size;
            this->primitive_base_t::load(size);

            if (0 == size)
            {
                return;
            }

            if (size != _Size)
            {
                boost::serialization::throw_exception(
                    InArchive_exception());
            }

            this->primitive_base_t::load_binary(bytes.data(), _Size);

            if constexpr (std::endian::native == std::endian::big)
            {
                std::ranges::reverse(bytes);
            }
        }

        // default fall through for any types not specified here
        template<class T>
            requires std::is_arithmetic_v<T>
        void load(T& t)
        {
            auto bytes = std::array<uint8_t, sizeof(T)>();
            load_impl(bytes);
            t = std::bit_cast<T>(bytes);
        }
        void load(boost::serialization::item_version_type& t)
        {
            boost::intmax_t l;
            load(l);
            // use cast to avoid compile time warning
            t = boost::serialization::item_version_type(l);
        }
        void load(boost::serialization::version_type& t)
        {
            boost::intmax_t l;
            load(l);
            // use cast to avoid compile time warning
            t = boost::serialization::version_type(l);
        }
        void load(boost::archive::class_id_type& t)
        {
            boost::int_least16_t l;
            load(l);
            // use cast to avoid compile time warning
            t = boost::archive::class_id_type(l);
        }
        template<class T>
            requires std::negation_v<std::is_arithmetic<T>>
        void load(T& t)
        {
            boost::intmax_t l;
            load(l);
            // use cast to avoid compile time warning
            t = T(l);
        }
        void load(std::string& t)
        {
            this->primitive_base_t::load(t);
        }
#ifndef BOOST_NO_STD_WSTRING
        void load(std::wstring& t)
        {
            this->primitive_base_t::load(t);
        }
#endif
        typedef boost::archive::detail::common_iarchive<InArchive>
            detail_common_iarchive;
        template<class T>
        void load_override(T& t)
        {
            this->detail_common_iarchive::load_override(t);
        }
        void load_override(boost::archive::class_name_type& t);
        // binary files don't include the optional information
        void load_override(boost::archive::class_id_optional_type&)
        {
        }

        void init(unsigned int flags);

    public:
        InArchive(std::istream& is, unsigned flags = 0) :
            primitive_base_t(
                *is.rdbuf(),
                0 != (flags & boost::archive::no_codecvt)),
            archive_base_t(flags),
            m_flags(0)
        {
            init(flags);
        }

        InArchive(
            std::basic_streambuf<
                std::istream::char_type,
                std::istream::traits_type>& bsb,
            unsigned int                    flags) :
            primitive_base_t(
                bsb,
                0 != (flags & boost::archive::no_codecvt)),
            archive_base_t(flags),
            m_flags(0)
        {
            init(flags);
        }
    };

    /////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
    // exception to be thrown if integer read from archive doesn't fit
    // variable being loaded
    class OutArchive_exception : public boost::archive::archive_exception
    {
    public:
        typedef enum
        {
            invalid_flags
        } exception_code;
        OutArchive_exception(exception_code c = invalid_flags)
        {
        }
        virtual const char* what() const throw()
        {
            const char* msg = "programmer error";
            switch (code)
            {
            case invalid_flags:
                msg = "cannot be both big and little endian";
            default:
                boost::archive::archive_exception::what();
            }
            return msg;
        }
    };

    /////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
    // "Portable" output binary archive.  This is a variation of the native binary
    // archive. it addresses integer size and endianness so that binary archives can
    // be passed across systems. Note:floating point types not addressed here

    class OutArchive : public boost::archive::basic_binary_oprimitive<
                           OutArchive,
                           std::ostream::char_type,
                           std::ostream::traits_type>,
                       public boost::archive::detail::common_oarchive<
                           OutArchive>
    {
        typedef boost::archive::basic_binary_oprimitive<
            OutArchive,
            std::ostream::char_type,
            std::ostream::traits_type>
            primitive_base_t;
        typedef boost::archive::detail::common_oarchive<
            OutArchive>
            archive_base_t;
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
    public:
#else
        friend archive_base_t;
        friend primitive_base_t; // since with override save below
        friend class boost::archive::detail::interface_oarchive<
            OutArchive>;
        friend class boost::archive::save_access;

    protected:
#endif
        unsigned int m_flags;

        template<size_t _Size>
        void save_impl(std::array<uint8_t, _Size> bytes)
        {
            if constexpr (std::endian::native == std::endian::big)
            {
                std::ranges::reverse(bytes);
            }

            this->primitive_base_t::save(uint8_t(_Size));
            this->primitive_base_t::save_binary(bytes.data(), _Size);
        }
        // add base class to the places considered when matching
        // save function to a specific set of arguments.  Note, this didn't
        // work on my MSVC 7.0 system so we use the sure-fire method below
        // using archive_base_t::save;

        // default fall through for any types not specified here
        template<class T>
            requires std::is_arithmetic_v<T>
        void save(T t)
        {
            auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(t);
            save_impl(bytes);
        }
        template<class T>
            requires std::negation_v<std::is_arithmetic<T>>
        void save(T t)
        {
            save(boost::intmax_t(t));
        }
        void save(const std::string& t)
        {
            this->primitive_base_t::save(t);
        }
#ifndef BOOST_NO_STD_WSTRING
        void save(const std::wstring& t)
        {
            this->primitive_base_t::save(t);
        }
#endif
        // default processing - kick back to base class.  Note the
        // extra stuff to get it passed borland compilers
        typedef boost::archive::detail::common_oarchive<OutArchive>
            detail_common_oarchive;
        template<class T>
        void save_override(T& t)
        {
            this->detail_common_oarchive::save_override(t);
        }
        // explicitly convert to char * to avoid compile ambiguities
        void save_override(const boost::archive::class_name_type& t)
        {
            const std::string s(t);
            *this << s;
        }
        // binary files don't include the optional information
        void save_override(
            const boost::archive::class_id_optional_type& /* t */
        )
        {
        }

        void init(unsigned int flags);

    public:
        OutArchive(std::ostream& os, unsigned flags = 0) :
            primitive_base_t(
                *os.rdbuf(),
                0 != (flags & boost::archive::no_codecvt)),
            archive_base_t(flags),
            m_flags(flags)
        {
            init(flags);
        }

        OutArchive(
            std::basic_streambuf<
                std::ostream::char_type,
                std::ostream::traits_type>& bsb,
            unsigned int                    flags) :
            primitive_base_t(
                bsb,
                0 != (flags & boost::archive::no_codecvt)),
            archive_base_t(flags),
            m_flags(0)
        {
            init(flags);
        }
    };
} // namespace Neon::IO

// required by export in boost version > 1.34
#ifdef BOOST_SERIALIZATION_REGISTER_ARCHIVE
BOOST_SERIALIZATION_REGISTER_ARCHIVE(Neon::IO::InArchive)
BOOST_SERIALIZATION_REGISTER_ARCHIVE(Neon::IO::OutArchive)
#endif

// required by export in boost <= 1.34
#define BOOST_ARCHIVE_CUSTOM_IARCHIVE_TYPES Neon::IO::InArchive
#define BOOST_ARCHIVE_CUSTOM_OARCHIVE_TYPES Neon::IO::OutArchive

//

#if defined(_MSC_VER)
#pragma warning(pop)
#endif