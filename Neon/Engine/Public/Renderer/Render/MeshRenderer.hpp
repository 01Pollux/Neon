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
        static constexpr uint32_t MATERIAL_FLAG_ALBEDO_MAP   = (1 << 0);
        static constexpr uint32_t MATERIAL_FLAG_NORMAL_MAP   = (1 << 1);
        static constexpr uint32_t MATERIAL_FLAG_SPECULAR_MAP = (1 << 2);
        static constexpr uint32_t MATERIAL_FLAG_EMISSIVE_MAP = (1 << 3);

        struct PerObjectData
        {
            Matrix4x4 World;
        };

        struct PerMaterialData
        {
            Vector3  Albedo;
            uint32_t AlbedoMapIndex;

            Vector3  Specular;
            uint32_t SpecularMapIndex;

            Vector3  Emissive;
            uint32_t EmissiveMapIndex;

            uint32_t NormalMapIndex;

            // MATERIAL_FLAG_*
            uint32_t Flags;
        };

    public:
        MeshRenderer();

        NEON_CLASS_NO_COPYMOVE(MeshRenderer);

        ~MeshRenderer() override;

        /// <summary>
        /// Render all sprites in the scene
        /// </summary>
        void Render(
            RHI::GpuResourceHandle CameraBuffer,
            RHI::ICommandList*     CommandList) override;

    private:
        flecs::query<
            Scene::Component::Transform,
            Scene::Component::MeshInstance>
            m_MeshQuery;
    };
} // namespace Neon::Renderer