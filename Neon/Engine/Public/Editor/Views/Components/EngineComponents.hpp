#pragma once

#include <Editor/Views/Component.hpp>

#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Physics.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>

namespace Neon::Editor
{
    class CameraComponentHandler : public IEditorComponentHandler
    {
    public:
        bool Draw(
            const flecs::entity& Entity,
            const flecs::id&     ComponentId) override;
    };

    class PhysicsComponentHandler : public IEditorComponentHandler
    {
    public:
        bool Draw(
            const flecs::entity& Entity,
            const flecs::id&     ComponentId) override;
    };

    class SpriteComponentHandler : public IEditorComponentHandler
    {
    public:
        bool Draw(
            const flecs::entity& Entity,
            const flecs::id&     ComponentId) override;
    };

    class TransformComponentHandler : public IEditorComponentHandler
    {
    public:
        bool Draw(
            const flecs::entity& Entity,
            const flecs::id&     ComponentId) override;
    };
} // namespace Neon::Editor