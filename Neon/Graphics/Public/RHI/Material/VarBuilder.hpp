#pragma once

#include <Utils/Struct.hpp>
#include <RHI/Resource/View.hpp>

namespace Neon::RHI
{
    class MaterialVarBuilder
    {
        using ResourceList = std::list<StringU8>;
        using SamplerList  = std::list<StringU8>;

    public:
        struct LayoutDescriptor
        {
            Structured::LayoutBuilder LayoutBuilder;
            ResourceList              Resources;
            SamplerList               Samplers;
        };

    public:
        /// <summary>
        /// Set data layout
        /// </summary>
        MaterialVarBuilder& SetData(
            Structured::LayoutBuilder Builder,
            bool                      Local)
        {
            auto& Layout         = Local ? m_LocalLayout : m_SharedLayout;
            Layout.LayoutBuilder = std::move(Builder);
            return *this;
        }

    public:
        /// <summary>
        /// Add resource to be used later when dispatching material
        /// </summary>
        auto& AddResource(
            StringU8 ResourceName,
            bool     Local)
        {
            auto& Layout = Local ? m_LocalLayout : m_SharedLayout;
            Layout.Resources.emplace_back(std::move(ResourceName));
            return *this;
        }

        /// <summary>
        /// Remove resource that was declaqred
        /// </summary>
        auto& RemoveResource(
            const StringU8& ResourceName,
            bool            Local)
        {
            auto& Layout = Local ? m_LocalLayout : m_SharedLayout;
            Layout.Resources.remove(ResourceName);
            return *this;
        }

    public:
        /// <summary>
        /// Add resource to be used later when dispatching material
        /// </summary>
        auto& AddSampler(
            StringU8 SamplerName,
            bool     Local)
        {
            auto& Layout = Local ? m_LocalLayout : m_SharedLayout;
            Layout.Samplers.emplace_back(std::move(SamplerName));
            return *this;
        }

        /// <summary>
        /// Remove resource that was created/imported
        /// </summary>
        auto& RemoveSampler(
            const StringU8& SamplerName,
            bool            Local)
        {
            auto& Layout = Local ? m_LocalLayout : m_SharedLayout;
            Layout.Samplers.remove(SamplerName);
            return *this;
        }

    public:
        /// <summary>
        /// Get shared layout
        /// </summary>
        const auto& SharedLayout() const noexcept
        {
            return m_SharedLayout;
        }

        /// <summary>
        /// Get local layout
        /// </summary>
        /// <returns></returns>
        const auto& LocalLayout() const noexcept
        {
            return m_LocalLayout;
        }

    private:
        LayoutDescriptor m_SharedLayout, m_LocalLayout;
    };
} // namespace Neon::RHI