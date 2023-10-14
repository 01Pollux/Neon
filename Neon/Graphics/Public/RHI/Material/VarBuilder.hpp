#pragma once

#include <Utils/Struct.hpp>
#include <RHI/Resource/View.hpp>

namespace Neon::RHI
{
    class MaterialVarBuilder
    {
        using ResourceMap = std::map<StringU8, RHI::DescriptorViewDesc>;
        using SamplerMap  = std::map<StringU8, RHI::SamplerDesc>;

        struct LayoutDescriptor
        {
            Structured::Layout Layout;
            ResourceMap        Resources;
            SamplerMap         Samplers;
        };

    public:
        /// <summary>
        /// Simple helper for constant buffer layout
        /// </summary>
        [[nodiscard]] static Structured::LayoutBuilder SharedDataBuilder()
        {
            return Structured::LayoutBuilder(16);
        }

        /// <summary>
        /// Simple helper for structured buffer
        /// </summary>
        [[nodiscard]] static Structured::LayoutBuilder LocalDataBuilder()
        {
            return Structured::LayoutBuilder(4);
        }

    public:
        /// <summary>
        /// Get shared data
        /// </summary>
        const auto& SharedData() const noexcept
        {
            return m_SharedLayout.Layout;
        }

        /// <summary>
        /// Get shared data
        /// </summary>
        auto& SharedData() noexcept
        {
            return m_SharedLayout.Layout;
        }

        /// <summary>
        /// Set local data
        /// </summary>
        void SetSharedData(
            const Structured::LayoutBuilder& Builder);

        /// <summary>
        /// Get local data
        /// </summary>
        const auto& LocalData() const noexcept
        {
            return m_LocalLayout.Layout;
        }

        /// <summary>
        /// Get local data
        /// </summary>
        auto& LocalData() noexcept
        {
            return m_LocalLayout.Layout;
        }

        /// <summary>
        /// Set local data
        /// </summary>
        void SetLocalData(
            const Structured::LayoutBuilder& Builder);

    public:
        /// <summary>
        /// Add resource to be used later when dispatching material
        /// </summary>
        void AddResource(
            const StringU8&                ResourceName,
            const RHI::DescriptorViewDesc& ViewDesc = {})
        {
            m_SharedLayout.Resources.emplace(ResourceName, ViewDesc);
        }

        /// <summary>
        /// Remove resource that was created/imported
        /// </summary>
        void RemoveResource(
            const StringU8& ResourceName)
        {
            m_SharedLayout.Resources.erase(ResourceName);
        }

    public:
        /// <summary>
        /// Add resource to be used later when dispatching material
        /// </summary>
        void AddSampler(
            const StringU8&         SamplerName,
            const RHI::SamplerDesc& Desc)
        {
            m_SharedLayout.Samplers.emplace(SamplerName, Desc);
        }

        /// <summary>
        /// Remove resource that was created/imported
        /// </summary>
        void RemoveSampler(
            const StringU8& SamplerName)
        {
            m_SharedLayout.Samplers.erase(SamplerName);
        }

    private:
        LayoutDescriptor m_SharedLayout, m_LocalLayout;
    };
} // namespace Neon::RHI