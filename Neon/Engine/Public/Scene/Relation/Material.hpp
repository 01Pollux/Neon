#pragma once

namespace Neon::Renderer
{
    class IMaterial;
} // namespace Neon::Renderer

namespace Neon::Scene::Relation
{
    struct GroupByMaterialInstance
    {
        struct Value
        {
            Renderer::IMaterial* MaterialInstance = nullptr;
        };
    };
} // namespace Neon::Scene::Relation