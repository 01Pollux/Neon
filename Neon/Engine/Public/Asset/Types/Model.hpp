#pragma once

#include <Asset/Asset.hpp>
#include <Mdl/Model.hpp>

namespace Neon::Asset
{
    class ModelAsset : public IAsset
    {
    public:
        class Handler;

        ModelAsset(
            Ptr<Mdl::Model> Model,
            const Handle&        AssetGuid,
            StringU8             Path) :
            IAsset(AssetGuid, Path),
            m_Model(std::move(Model))
        {
        }

        /// <summary>
        /// Get the model from this asset.
        /// </summary>
        [[nodiscard]] const Ptr<Mdl::Model>& GetModel() const
        {
            return m_Model;
        }

    private:
        Ptr<Mdl::Model> m_Model;
    };
} // namespace Neon::Asset