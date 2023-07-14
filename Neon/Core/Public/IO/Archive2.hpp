#pragma once

#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>

namespace Neon::IO
{
    using InArchive2  = boost::archive::polymorphic_iarchive;
    using OutArchive2 = boost::archive::polymorphic_oarchive;
} // namespace Neon::IO