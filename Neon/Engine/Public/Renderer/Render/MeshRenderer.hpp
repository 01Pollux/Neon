#pragma once

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Mesh.hpp>

#include <RHI/FrameResource.hpp>

namespace Neon::Renderer
{
    class MeshRenderer
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

        ~MeshRenderer();

        /// <summary>
        /// Render all sprites in the scene
        /// </summary>
        void Render(
            RHI::GpuResourceHandle CameraBuffer,
            RHI::ICommandList*     CommandList);

        struct FrameResource
        {
            Ptr<RHI::IGpuResource> PerObjectBuffer;
            Ptr<RHI::IGpuResource> PerMaterialBuffer;

            FrameResource();
        };

    private:
        flecs::query<
            Scene::Component::Transform,
            Scene::Component::MeshInstance>
            m_MeshQuery;

        RHI::FrameResource<FrameResource> m_MeshData;
    };
} // namespace Neon::Renderer