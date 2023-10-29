#pragma once

#include <Scene/CSG/Brush.hpp>
#include <boost/serialization/vector.hpp>

namespace boost::serialization
{
    class access;
} // namespace boost::serialization

namespace Neon::Scene::CSG
{
    class Shape
    {

    public:
        /// <summary>
        /// Get the brush of the shape.
        /// </summary>
        [[nodiscard]] const Brush& GetBrush() const
        {
            return m_Brush;
        }

    private:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive&          Archive,
            const unsigned int Version)
        {
            Archive& m_Brush;
        }

    protected:
        Shape() = default;

    protected:
        Brush m_Brush;
    };

    class Box3D : public Shape
    {
    public:
        Box3D(
            const Vector3&             Size     = Vec::One<Vector3>,
            const Ptr<RHI::IMaterial>& Material = nullptr);

        /// <summary>
        /// Get the size of the box.
        /// </summary>
        [[nodiscard]] const Vector3& GetSize() const
        {
            return m_Size;
        }

        /// <summary>
        /// Set the size of the box.
        /// </summary>
        void SetSize(const Vector3& Size)
        {
            m_Size = Size;
        }

        /// <summary>
        /// Get the material of the box.
        /// </summary>
        [[nodiscard]] const Ptr<RHI::IMaterial>& GetMaterial() const
        {
            return m_Material;
        }

        /// <summary>
        /// Set the material of the box.
        /// </summary>
        void SetMaterial(
            const Ptr<RHI::IMaterial>& Material)
        {
            m_Material = Material;
        }

        /// <summary>
        /// Rebuild the brush.
        /// </summary>
        void Rebuild();

    private:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t)
        {
            Archive& boost::serialization::base_object<Shape>(*this);
            Archive& m_Size;
            // TODO: Add material serialization
            // Archive& m_Material;
        }

    private:
        Vector3             m_Size;
        Ptr<RHI::IMaterial> m_Material;
    };

    //

    class Sphere : public Shape
    {
    public:
        Sphere(
            float                      Radius      = 0.5f,
            uint32_t                   RingCount   = 16,
            uint32_t                   SectorCount = 16,
            const Ptr<RHI::IMaterial>& Material    = nullptr);

        /// <summary>
        /// Get the radius of the sphere.
        /// </summary>
        [[nodiscard]] float GetRadius() const
        {
            return m_Radius;
        }

        /// <summary>
        /// Set the radius of the sphere.
        /// </summary>
        void SetRadius(float Radius)
        {
            m_Radius = Radius;
        }

        /// <summary>
        /// Rebuild the brush.
        /// </summary>
        void Rebuild();

    private:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t)
        {
            Archive& boost::serialization::base_object<Shape>(*this);
            Archive& m_Radius;
            Archive& m_RingCount;
            Archive& m_SectorCount;
        }

    private:
        float    m_Radius;
        uint32_t m_RingCount;
        uint32_t m_SectorCount;

        Ptr<RHI::IMaterial> m_Material;
    };
} // namespace Neon::Scene::CSG