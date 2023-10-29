#pragma once

#include <Core/Neon.hpp>

#include <Mdl/Mesh.hpp>
#include <Math/Transform.hpp>

#include <vector>

namespace Neon::RHI
{
    class IMaterial;
} // namespace Neon::RHI

namespace Neon::Scene::CSG
{
    class Brush
    {
    public:
        enum class Operation : uint8_t
        {
            Union,
            Subtraction,
            Intersection
        };

    public:
        Brush() = default;

        Brush(
            Mdl::Mesh Target) :
            m_Target(Target),
            m_FinalMesh(std::move(Target))
        {
        }

        Brush(
            Mdl::Mesh Target,
            Mdl::Mesh Brush,
            Operation Op) :
            m_Target(std::move(Target)),
            m_Brush(std::move(Brush)),
            m_Operation(Op)
        {
            Rebuild();
        }

        Brush(
            Mdl::Mesh    Target,
            const Brush& Brush_,
            Operation    Op) :
            m_Target(Target),
            m_Brush(Brush_.m_FinalMesh),
            m_Operation(Op)
        {
            Rebuild();
        }

        Brush(
            const Brush& Target,
            const Brush& Brush_,
            Operation    Op) :
            m_Target(Target.m_FinalMesh),
            m_Brush(Brush_.m_FinalMesh),
            m_Operation(Op)
        {
            Rebuild();
        }

    public:
        /// <summary>
        /// Get the first input mesh of the brush.
        /// </summary>
        [[nodiscard]] const auto& GetTarget() const
        {
            return m_Target;
        }

        /// <summary>
        /// Set the first input mesh of the brush.
        /// </summary>
        [[nodiscard]] void SetTarget(
            Mdl::Mesh Target)
        {
            m_Target = std::move(Target);
        }

        /// <summary>
        /// Get the second input mesh of the brush.
        /// </summary>
        [[nodiscard]] const auto& GetBrush() const
        {
            return m_Brush;
        }

        /// <summary>
        /// Set the second input mesh of the brush.
        /// </summary>
        [[nodiscard]] void SetBrush(
            Mdl::Mesh Brush_)
        {
            m_Brush = std::move(Brush_);
        }

        /// <summary>
        /// Set the second input mesh of the brush.
        /// </summary>
        [[nodiscard]] void SetBrush(
            const Brush& Brush_)
        {
            m_Brush = Brush_.m_FinalMesh;
        }

        /// <summary>
        /// Get the final mesh of the brush.
        /// </summary>
        [[nodiscard]] const auto& GetMesh() const
        {
            return m_FinalMesh;
        }

        /// <summary>
        /// Get operation of the brush.
        /// </summary>
        Operation GetOperation() const noexcept
        {
            return m_Operation;
        }

        /// <summary>
        /// Get operation of the brush.
        /// </summary>
        void SetOperation(
            Operation Op) noexcept
        {
            m_Operation = Op;
        }

        /// <summary>
        /// Rebuild the brush.
        /// </summary>
        void Rebuild();

    public:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive&          Archive,
            const unsigned int Version)
        {
            // TODO: add material serialization
        }

    private:
        Mdl::Mesh m_Target, m_Brush, m_FinalMesh;
        Operation m_Operation = Operation::Union;
    };
} // namespace Neon::Scene::CSG