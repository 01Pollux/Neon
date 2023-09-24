#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/Descriptor.hpp>
#include <RHI/Commands/Common.hpp>

#include <map>
#include <vector>

namespace Neon::RHI
{
    class ICommandList;
    class IPipelineState;
    class IRootSignature;

    template<bool>
    class GenericMaterialBuilder;

    //

    class IMaterial
    {
    public:
        enum class PipelineVariant : uint8_t
        {
            /// <summary>
            /// Depth prepass.
            /// Used for forward+ rendering.
            /// </summary>
            DepthPass,

            /// <summary>
            /// Render pass.
            /// Used for forward+ rendering.
            RenderPass,

            /// <summary>
            /// Compute pass.
            /// </summary>
            ComputePass = RenderPass,

            Count
        };

        using PipelineStateList = std::array<Ptr<IPipelineState>, size_t(PipelineVariant::Count)>;

        static Ptr<IMaterial> Create(
            const wchar_t*                       Name,
            const GenericMaterialBuilder<false>& Builder);

        static Ptr<IMaterial> Create(
            const wchar_t*                      Name,
            const GenericMaterialBuilder<true>& Builder);

        virtual ~IMaterial() = default;

        /// <summary>
        /// Creates a new material instance.
        /// </summary>
        [[nodiscard]] virtual Ptr<IMaterial> CreateInstance() = 0;

        /// <summary>
        /// Get the root signature.
        /// </summary>
        const Ptr<IRootSignature>& GetRootSignature() const noexcept
        {
            return m_RootSignature;
        }

        /// <summary>
        /// Get pipeline state.
        /// </summary>
        const Ptr<IPipelineState>& GetPipelineState(
            PipelineVariant Variant) const noexcept
        {
            return m_PipelineStates[static_cast<uint8_t>(Variant)];
        }

    protected:
        Ptr<IRootSignature> m_RootSignature;
        PipelineStateList   m_PipelineStates;
    };
} // namespace Neon::RHI