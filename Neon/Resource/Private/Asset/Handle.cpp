#include <ResourcePCH.hpp>
#include <Asset/Handle.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_hash.hpp>

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

namespace std
{
    size_t hash<Neon::AAsset::Handle>::operator()(
        const Neon::AAsset::Handle& Value) const noexcept
    {
        return boost::uuids::hash_value(Value);
    };
} // namespace std
