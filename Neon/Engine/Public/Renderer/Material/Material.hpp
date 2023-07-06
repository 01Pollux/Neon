#pragma once

#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Resource/Resource.hpp>
#include <Renderer/Material/Common.hpp>

namespace Neon::Renderer
{
    class MaterialInstance;

    template<bool>
    class GenericMaterialBuilder;

    class IMaterial : public std::enable_shared_from_this<IMaterial>
    {
    public:
        static Ptr<IMaterial> Create(
            RHI::ISwapchain*                     Swapchain,
            const GenericMaterialBuilder<false>& Builder);

        static Ptr<IMaterial> Create(
            RHI::ISwapchain*                    Swapchain,
            const GenericMaterialBuilder<true>& Builder);

        virtual ~IMaterial() = default;

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] virtual Ptr<MaterialInstance> CreateInstance() = 0;
    };

    class IMaterialInstance
    {
    public:
        virtual ~IMaterialInstance() = default;

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] virtual Ptr<MaterialInstance> CreateInstance() = 0;

        /// <summary>
        /// Get the parent material.
        /// </summary>
        [[nodiscard]] const Ptr<IMaterial>& GetParentMaterial() const
        {
            return m_ParentMaterial;
        }

    private:
        Ptr<IMaterial> m_ParentMaterial;
    };
} // namespace Neon::Renderer