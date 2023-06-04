#include <CorePCH.hpp>
#include <ostream>
#include <IO/Archive.hpp>
#include <boost/predef/other/endian.h>

// (C) Copyright 2002-7 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.
namespace Neon::IO
{
    void OutArchive::init(unsigned int flags)
    {
        if (0 == (flags & boost::archive::no_header))
        {
            // write signature in an archive version independent manner
            const std::string file_signature(
                boost::archive::BOOST_ARCHIVE_SIGNATURE());
            *this << file_signature;
            // write library version
            const boost::archive::library_version_type v(
                boost::archive::BOOST_ARCHIVE_VERSION());
            *this << v;
        }
        save(static_cast<unsigned char>(m_flags >> CHAR_BIT));
    }
} // namespace Neon::IO

#include <boost/archive/impl/archive_serializer_map.ipp>
#include <boost/archive/impl/basic_binary_oprimitive.ipp>

namespace boost::archive
{
    namespace detail
    {
        template class archive_serializer_map<Neon::IO::OutArchive>;
    }

    template class basic_binary_oprimitive<
        Neon::IO::OutArchive,
        std::ostream::char_type,
        std::ostream::traits_type>;
} // namespace boost::archive