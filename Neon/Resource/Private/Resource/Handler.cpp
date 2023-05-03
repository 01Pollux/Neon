#include <ResourcePCH.hpp>
#include <Resource/Handler.hpp>

namespace Neon::Asset
{
    void AssetResourceHandlers::Append(
        UPtr<IAssetResourceHandler> Handler)
    {
        m_Handlers[typeid(*Handler).hash_code()] = std::move(Handler);
    }

    IAssetResourceHandler* AssetResourceHandlers::Get(
        size_t LoaderId) const noexcept
    {
        auto Iter = m_Handlers.find(LoaderId);
        return Iter == m_Handlers.end() ? nullptr : Iter->second.get();
    }
} // namespace Neon::Asset