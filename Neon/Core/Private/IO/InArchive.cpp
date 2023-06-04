#include <CorePCH.hpp>
#include <istream>
#include <ostream>
#include <IO/Archive.hpp>

namespace Neon::IO
{
    void InArchive::load_override(
        boost::archive::class_name_type& t)
    {
        std::string cn;
        cn.reserve(BOOST_SERIALIZATION_MAX_KEY_SIZE);
        load_override(cn);
        if (cn.size() > (BOOST_SERIALIZATION_MAX_KEY_SIZE - 1))
        {
            boost::serialization::throw_exception(
                boost::archive::archive_exception(
                    boost::archive::archive_exception::invalid_class_name));
        }
        std::memcpy(t, cn.data(), cn.size());
        // borland tweak
        t.t[cn.size()] = '\0';
    }

    void InArchive::init(unsigned int flags)
    {
        if (0 == (flags & boost::archive::no_header))
        {
            // read signature in an archive version independent manner
            std::string file_signature;
            *this >> file_signature;
            if (file_signature != boost::archive::BOOST_ARCHIVE_SIGNATURE())
                boost::serialization::throw_exception(
                    boost::archive::archive_exception(
                        boost::archive::archive_exception::invalid_signature));
            // make sure the version of the reading archive library can
            // support the format of the archive being read
            boost::archive::library_version_type input_library_version;
            *this >> input_library_version;

            // extra little .t is to get around borland quirk
            if (boost::archive::BOOST_ARCHIVE_VERSION() < input_library_version)
                boost::serialization::throw_exception(
                    boost::archive::archive_exception(
                        boost::archive::archive_exception::unsupported_version));

#if BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3205))
            this->set_library_version(input_library_version);
            // #else
            // #if ! BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
            // detail::
            // #endif
            boost::archive::detail::basic_iarchive::set_library_version(
                input_library_version);
#endif
        }
        unsigned char x;
        load(x);
        m_flags = x << CHAR_BIT;
    }
} // namespace Neon::IO

#include <boost/archive/impl/archive_serializer_map.ipp>
#include <boost/archive/impl/basic_binary_iprimitive.ipp>

namespace boost::archive
{
    namespace detail
    {
        template class archive_serializer_map<Neon::IO::InArchive>;
    }

    template class basic_binary_iprimitive<
        Neon::IO::InArchive,
        std::istream::char_type,
        std::istream::traits_type>;
} // namespace boost::archive