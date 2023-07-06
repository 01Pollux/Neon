#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Common.hpp>

namespace Neon::RHI
{
    class ISwapchain;
    class IGraphicsCommandList;
} // namespace Neon::RHI

namespace Neon::Renderer
{
    class IMaterialInstance;

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
        [[nodiscard]] virtual Ptr<IMaterialInstance> CreateInstance() = 0;

        /// <summary>
        /// Get the default material instance.
        /// </summary>
        [[nodiscard]] virtual const Ptr<IMaterialInstance>& GetDefaultInstance() = 0;

        /// <summary>
        /// Bind the material to the command list.
        /// </summary>
        virtual void Bind(
            RHI::IGraphicsCommandList* CommandList) = 0;
    };

    class IMaterialInstance
    {
    public:
        virtual ~IMaterialInstance() = default;

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] virtual Ptr<IMaterialInstance> CreateInstance() = 0;

        /// <summary>
        /// Get the intsance primitive topology.
        /// </summary>
        [[nodiscard]] RHI::PrimitiveTopology GetTopology();

        /// <summary>
        /// Set the intsance primitive topology.
        /// </summary>
        [[nodiscard]] void SetTopology(
            RHI::PrimitiveTopology Topology);

        /// <summary>
        /// Get the parent material.
        /// </summary>
        [[nodiscard]] const Ptr<IMaterial>& GetParentMaterial() const;

    protected:
        Ptr<IMaterial>         m_ParentMaterial;
        RHI::PrimitiveTopology m_Topology;
    };
} // namespace Neon::Renderer