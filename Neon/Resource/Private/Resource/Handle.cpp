#include <ResourcePCH.hpp>
#include <Resource/Handle.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/random_generator.hpp>

namespace Neon::Asset
{
    std::string AssetHandle::ToString() const
    {
        return boost::uuids::to_string(*this);
    }

    std::wstring AssetHandle::ToWideString() const
    {
        return boost::uuids::to_wstring(*this);
    }

    AssetHandle AssetHandle::FromString(
        const std::string& Id)
    {
        return { boost::uuids::string_generator()(Id) };
    }

    AssetHandle AssetHandle::Random()
    {
        return { boost::uuids::random_generator()() };
    }
} // namespace Neon::Asset