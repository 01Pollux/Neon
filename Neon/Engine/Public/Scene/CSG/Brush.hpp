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
            Mdl::Mesh Mesh0) :
            m_Mesh0(Mesh0),
            m_FinalMesh(std::move(Mesh0))
        {
        }

        Brush(
            Mdl::Mesh Mesh0,
            Mdl::Mesh Mesh1,
            Operation Op) :
            m_Mesh0(std::move(Mesh0)),
            m_Mesh1(std::move(Mesh1)),
            m_Operation(Op)
        {
            Rebuild();
        }

        Brush(
            Mdl::Mesh    Mesh0,
            const Brush& Br1,
            Operation    Op) :
            m_Mesh0(Mesh0),
            m_Mesh1(Br1.m_FinalMesh),
            m_Operation(Op)
        {
            Rebuild();
        }

        Brush(
            const Brush& Br0,
            const Brush& Br1,
            Operation    Op) :
            m_Mesh0(Br0.m_FinalMesh),
            m_Mesh1(Br1.m_FinalMesh),
            m_Operation(Op)
        {
            Rebuild();
        }

    public:
        /// <summary>
        /// Get the first input mesh of the brush.
        /// </summary>
        [[nodiscard]] const auto& GetMesh0() const
        {
            return m_Mesh0;
        }

        /// <summary>
        /// Set the first input mesh of the brush.
        /// </summary>
        [[nodiscard]] void SetMesh0(
            Mdl::Mesh Mesh)
        {
            m_Mesh0 = std::move(Mesh);
        }

        /// <summary>
        /// Get the second input mesh of the brush.
        /// </summary>
        [[nodiscard]] const auto& GetMesh1() const
        {
            return m_Mesh1;
        }

        /// <summary>
        /// Set the second input mesh of the brush.
        /// </summary>
        [[nodiscard]] void SetMesh1(
            Mdl::Mesh Mesh)
        {
            m_Mesh1 = std::move(Mesh);
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
        Mdl::Mesh m_Mesh0, m_Mesh1, m_FinalMesh;
        Operation m_Operation = Operation::Union;
    };
} // namespace Neon::Scene::CSG