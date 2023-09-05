#pragma once

#include <Asset/Asset.hpp>
#include <Renderer/Mesh/Mesh.hpp>

namespace Neon::Asset
{
    class ModelAsset : public IAsset
    {
    public:
        class Handler;

        ModelAsset(
            Ptr<Renderer::Model> Model,
            const Handle&        AssetGuid,
            StringU8             Path) :
            IAsset(AssetGuid, Path),
            m_Model(std::move(Model))
        {
        }

        /// <summary>
        /// Get the model from this asset.
        /// </summary>
        [[nodiscard]] const Ptr<Renderer::Model>& GetModel() const
        {
            return m_Model;
        }

    private:
        Ptr<Renderer::Model> m_Model;
    };
} // namespace Neon::Asset