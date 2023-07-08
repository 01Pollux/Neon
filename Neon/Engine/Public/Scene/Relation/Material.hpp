#pragma once

namespace Neon::Renderer
{
    class IMaterialInstance;
} // namespace Neon::Renderer

namespace Neon::Scene::Relation
{
    struct GroupByMaterialInstance
    {
        struct Value
        {
            Renderer::IMaterialInstance* MaterialInstance = nullptr;
        };
    };
} // namespace Neon::Scene::Relation