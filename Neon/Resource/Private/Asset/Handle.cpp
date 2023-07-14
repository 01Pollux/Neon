#include <ResourcePCH.hpp>
#include <Asset/Handle.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/random_generator.hpp>

namespace Neon::AAsset
{
    StringU8 Handle::ToString() const
    {
        return boost::uuids::to_string(*this);
    }

    String Handle::ToWideString() const
    {
        return boost::uuids::to_wstring(*this);
    }

    Handle Handle::FromString(
        const std::string& Id)
    {
        return { boost::uuids::string_generator()(Id) };
    }

    Handle Handle::Random()
    {
        return { boost::uuids::random_generator()() };
    }
} // namespace Neon::AAsset