#pragma once

#include <Renderer/Render/BaseRenderer.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>

namespace Neon::Renderer
{
    class MeshRenderer : public IRenderer
    {
    public:
        MeshRenderer();

        NEON_CLASS_NO_COPYMOVE(MeshRenderer);

        ~MeshRenderer() override;

        /// <summary>
        /// Render all sprites in the scene
        /// </summary>
        void Render(
            RHI::GpuResourceHandle     CameraBuffer,
            RHI::ICommandList* CommandList) override;

    private:
        flecs::query<
            Scene::Component::Transform,
            Scene::Component::MeshInstance>
            m_MeshQuery;
    };
} // namespace Neon::Renderer