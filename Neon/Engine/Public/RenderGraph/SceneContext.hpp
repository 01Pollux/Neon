#pragma once

#include <RenderGraph/Common.hpp>
#include <Math/Common.hpp>

namespace Neon::RG
{
    class GraphStorage;

    class SceneContext
    {
        struct alignas(16) PerMaterialData
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

        static constexpr uint32_t AlignOfPerMaterialData = uint32_t(alignof(PerMaterialData));
        static constexpr uint32_t SizeOfPerMaterialData  = uint32_t(Math::AlignUp(sizeof(PerMaterialData), AlignOfPerMaterialData));

    public:
        enum class RenderType : uint8_t
        {
            DepthPrepass,
            RenderPass
        };

        SceneContext(
            const GraphStorage& Storage);

        /// <summary>
        /// Render the scene depending on the type
        /// </summary>
        void Render(
            RHI::ICommandList*       CommandList,
            RenderType               Type,
            RHI::GpuDescriptorHandle OpaqueLightDataHandle,
            RHI::GpuDescriptorHandle TransparentLightDataHandle) const;

        /// <summary>
        /// Get the light's resource view
        /// </summary>
        [[nodiscard]] RHI::CpuDescriptorHandle GetLightsResourceView() const noexcept;

        /// <summary>
        /// Get the light's resource count
        /// </summary>
        [[nodiscard]] uint32_t GetLightsCount() const noexcept;

    private:
        const GraphStorage& m_Storage;
    };
} // namespace Neon::RG