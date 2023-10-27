#pragma once

#include <Core/Neon.hpp>
#include <Math/Vector.hpp>
#include <Math/Transform.hpp>
#include <vector>

namespace Neon::RHI
{
    class IMaterial;
}

namespace Neon::Scene
{
    class CSGBrush
    {
    public:
        struct Face
        {
            Vector3 Vertices[3];
            Vector2 UVs[3];
            int     MaterialIndex;
        };

        using FaceList     = std::vector<Face>;
        using MaterialList = std::vector<Ptr<RHI::IMaterial>>;

    public:
        CSGBrush() = default;
        CSGBrush(
            FaceList     Faces,
            MaterialList Materials);
        CSGBrush(
            const CSGBrush&        Brush,
            const TransformMatrix& Transform);

        /// <summary>
        /// Get faces of the brush.
        /// </summary>
        [[nodiscard]] const FaceList& GetFaces() const;

        /// <summary>
        /// Get materials of the brush.
        /// </summary>
        [[nodiscard]] const MaterialList& GetMaterials() const;

    private:
        FaceList     m_Faces;
        MaterialList m_Materials;
    };
} // namespace Neon::Scene