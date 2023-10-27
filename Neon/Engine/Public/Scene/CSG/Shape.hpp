#pragma once

#include <Scene/CSG/Brush.hpp>

namespace boost::serialization
{
    class access;
} // namespace boost::serialization

namespace Neon::Scene
{
    class CSGShape
    {

    public:
        /// <summary>
        /// Get the brush of the shape.
        /// </summary>
        [[nodiscard]] const CSGBrush& GetBrush() const
        {
            return m_Brush;
        }

    public:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive&          Archive,
            const unsigned int Version)
        {
            Archive& m_Brush;
        }

    protected:
        CSGShape() = default;

    protected:
        CSGBrush m_Brush;
    };

    class CSGBox3D : public CSGShape
    {
    public:
        CSGBox3D();

        CSGBox3D(
            const Vector3&             Size,
            const Ptr<RHI::IMaterial>& Material);

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

    public:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive&          Archive,
            const unsigned int Version)
        {
            Archive& boost::serialization::base_object<CSGShape>(*this);
            Archive& m_Size;
            Archive& m_Material;
        }

    private:
        Vector3             m_Size;
        Ptr<RHI::IMaterial> m_Material;
    };
} // namespace Neon::Scene