#include <EnginePCH.hpp>
#include <Scene/CSG/Brush.hpp>
#include <Math/Matrix.hpp>
#include <Mdl/Submesh.hpp>

#include <deque>
#include <Log/Logger.hpp>

namespace Neon::Scene::CSG
{
    struct ModelInfo
    {
        void* VertexBuffer;
        void* IndexBuffer;

        RHI::UBufferPoolHandle VertexBufferHandle;
        RHI::UBufferPoolHandle IndexBufferHandle;

        const Mdl::Mesh& Mesh;

        ModelInfo(
            const Mdl::Mesh& Mesh) :
            Mesh(Mesh)
        {
            auto& Model   = Mesh.GetModel();
            auto& Submesh = Mesh.GetData();

            std::tie(VertexBuffer, VertexBufferHandle) = Model->PeekVertexBuffer(
                Submesh.VertexOffset,
                Submesh.VertexCount);
            std::tie(IndexBuffer, IndexBufferHandle) = Model->PeekIndexBuffer(
                Submesh.IndexOffset,
                Submesh.IndexCount);
        }

        [[nodiscard]] uint32_t GetIndex(
            uint32_t Index) const
        {
            if (Mesh.GetModel()->HasSmallIndices())
            {
                return std::bit_cast<const uint16_t*>(IndexBuffer)[Index];
            }
            else
            {
                return std::bit_cast<const uint32_t*>(IndexBuffer)[Index];
            }
        }

        [[nodiscard]] const Mdl::MeshVertex GetVertex(
            uint32_t Index) const
        {
            return std::bit_cast<const Mdl::MeshVertex*>(VertexBuffer)[Index];
        }
    };

    //

    const float csgjs_EPSILON = 0.0001f;

    // Holds a node in a BSP tree. A BSP tree is built from a collection of polygons
    // by picking a polygon to split along. That polygon (and all other coplanar
    // polygons) are added directly to that node and the other polygons are added to
    // the front and/or back subtrees. This is not a leafy BSP tree since there is
    // no distinction between internal and leaf nodes.
    struct CSGNode
    {
        struct Polygon
        {
            std::vector<Mdl::MeshVertex> vertices;
            Math::Plane                  plane;

            Polygon() = default;
            Polygon(const std::vector<Mdl::MeshVertex>& vertices) :
                vertices(vertices), plane(vertices[0].Position, vertices[1].Position, vertices[2].Position)
            {
            }

            void flip()
            {
                std::reverse(vertices.begin(), vertices.end());
                for (size_t i = 0; i < vertices.size(); i++)
                {
                    vertices[i].Normal    = -vertices[i].Normal;
                    vertices[i].Tangent   = -vertices[i].Tangent;
                    vertices[i].Bitangent = -vertices[i].Bitangent;
                }
                plane.Flip();
            }
        };

        std::vector<Polygon> polygons;

        CSGNode*    front = nullptr;
        CSGNode*    back  = nullptr;
        Math::Plane plane;

        CSGNode() = default;
        CSGNode(const std::vector<Polygon>& list);
        ~CSGNode();

        CSGNode*             clone() const;
        void                 clipto(const CSGNode* other);
        void                 invert();
        void                 build(const std::vector<Polygon>& Polygon);
        std::vector<Polygon> clippolygons(const std::vector<Polygon>& list) const;
        std::vector<Polygon> allpolygons() const;

        static void splitpolygon(
            const Math::Plane& plane,
            const Polygon& poly, std::vector<Polygon>& coplanarFront,
            std::vector<Polygon>& coplanarBack, std::vector<Polygon>& front,
            std::vector<Polygon>& back);

        enum Classification
        {
            CLS_COPLANAR = 0,
            CLS_FRONT    = 1,
            CLS_BACK     = 2,
            CLS_SPANNING = 3
        };

        static Classification classify(
            const Math::Plane& plane,
            const Vector3&     p)
        {
            float t = glm::dot(plane.GetNormal(), p) + plane.w;
            if (t < -csgjs_EPSILON)
                return CLS_BACK;
            if (t > csgjs_EPSILON)
                return CLS_FRONT;
            return CLS_COPLANAR;
        }
    };

    using csg_function = CSGNode* (*)(const CSGNode* a1, const CSGNode* b1);

    std::vector<CSGNode::Polygon> csgjs_operation(
        const std::vector<CSGNode::Polygon>& apoly,
        const std::vector<CSGNode::Polygon>& bpoly,
        csg_function                         fun)
    {
        CSGNode A(apoly);
        CSGNode B(bpoly);

        /* create a unique pointer here so we can delete AB on exit */
        std::unique_ptr<CSGNode> AB(fun(&A, &B));
        return AB->allpolygons();
    }

    std::vector<CSGNode::Polygon> modeltopolygons(
        ModelInfo* model)
    {
        std::vector<CSGNode::Polygon> list;

        auto& submesh = model->Mesh.GetData();

        for (uint32_t i = 0; i < submesh.IndexCount; i += 3)
        {
            list.emplace_back(
                std::vector{ model->GetVertex(model->GetIndex(i + 0)),
                             model->GetVertex(model->GetIndex(i + 1)),
                             model->GetVertex(model->GetIndex(i + 2)) });
        }
        return list;
    }

    CSGNode* csg_subtract_fn(const CSGNode* a1, const CSGNode* b1)
    {
        CSGNode* a = a1->clone();
        CSGNode* b = b1->clone();
        a->invert();
        a->clipto(b);
        b->clipto(a);
        b->invert();
        b->clipto(a);
        b->invert();
        a->build(b->allpolygons());
        a->invert();
        CSGNode* ret = NEON_NEW CSGNode(a->allpolygons());
        delete a;
        delete b;
        return ret;
    }

    CSGNode* csg_union_fn(const CSGNode* a1, const CSGNode* b1)
    {
        CSGNode* a = a1->clone();
        CSGNode* b = b1->clone();
        a->clipto(b);
        b->clipto(a);
        b->invert();
        b->clipto(a);
        b->invert();
        a->build(b->allpolygons());
        CSGNode* ret = new CSGNode(a->allpolygons());
        delete a;
        delete b;
        return ret;
    }

    CSGNode* csg_intersect_fn(const CSGNode* a1, const CSGNode* b1)
    {
        CSGNode* a = a1->clone();
        CSGNode* b = b1->clone();
        a->invert();
        b->clipto(a);
        b->invert();
        a->clipto(b);
        b->clipto(a);
        a->build(b->allpolygons());
        a->invert();
        CSGNode* ret = new CSGNode(a->allpolygons());
        delete a;
        delete b;
        return ret;
    }

    //

    inline std::vector<CSGNode::Polygon> csgjs_operation(
        ModelInfo*   a,
        ModelInfo*   b,
        csg_function fun)
    {
        return csgjs_operation(modeltopolygons(a), modeltopolygons(b), fun);
    }

    auto csg_subtract(ModelInfo* a, ModelInfo* b)
    {
        return csgjs_operation(a, b, csg_subtract_fn);
    }

    auto csg_union(ModelInfo* a, ModelInfo* b)
    {
        return csgjs_operation(a, b, csg_union_fn);
    }

    auto csg_intersect(ModelInfo* a, ModelInfo* b)
    {
        return csgjs_operation(a, b, csg_intersect_fn);
    }

    //

    auto csg_to_mdl(
        const std::vector<CSGNode::Polygon>& Polygons,
        const Ptr<RHI::IMaterial>&           Material)
    {
        Mdl::Model::SubmeshList Submeshes{
            {}
        };
        Mdl::Model::MeshNodeList Nodes{
            Mdl::MeshNode{ .Name = "Merged", .Submeshes = { 0 } }
        };
        Mdl::Model::MaterialsTable Materials{
            Material
        };

        Vector3 Min(std::numeric_limits<float>::max()),
            Max(std::numeric_limits<float>::lowest());

        //

        std::vector<Mdl::MeshVertex> Vertices;
        std::vector<uint32_t>        Indices;

        Vertices.reserve(Polygons.size() * 3);
        Indices.reserve(Polygons.size() * 3);

        for (const auto& Poly : Polygons)
        {
            uint32_t Start = uint32_t(Vertices.size());

            uint32_t ai = uint32_t(Vertices.size());
            auto&    a  = Vertices.emplace_back(Poly.vertices[0]).Position;

            Min = glm::min(Min, Poly.vertices[0].Position);
            Max = glm::max(Max, Poly.vertices[0].Position);

            for (size_t j = 2; j < Poly.vertices.size(); j++)
            {
                uint32_t bi = uint32_t(Vertices.size());
                auto&    b  = Vertices.emplace_back(Poly.vertices[j - 1]).Position;

                uint32_t ci = uint32_t(Vertices.size());
                auto&    c  = Vertices.emplace_back(Poly.vertices[j]).Position;

                Min = glm::min(Min, Poly.vertices[j - 1].Position);
                Max = glm::max(Max, Poly.vertices[j].Position);

                if (a != b && b != c && c != a)
                {
                    Indices.push_back(ai);
                    Indices.push_back(bi);
                    Indices.push_back(ci);
                }
            }
        }

        size_t VertexCount  = Vertices.size();
        size_t IndexCount   = Indices.size();
        size_t VerticesSize = VertexCount * sizeof(Mdl::MeshVertex);
        bool   SmallIndices = VertexCount < std::numeric_limits<uint16_t>::max();
        size_t IndicesSize  = IndexCount * (SmallIndices ? sizeof(uint16_t) : sizeof(uint32_t));

        Submeshes[0].AABB = {
            .Extents = (Max - Min) * 0.5f,
        };
        Submeshes[0].IndexCount  = uint32_t(IndexCount);
        Submeshes[0].VertexCount = uint32_t(VertexCount);

        //

        auto PreferPooledBuffer = [](size_t Size)
        {
            return Size < size_t(0xFFFF * 1.f / 3.f);
        };

        //

        Mdl::Model::GPUBuffer VertexBuffer;
        Mdl::Model::GPUBuffer IndexBuffer;

        // if VerticesSize less than 0xFFFF / 3, we can use 16 bit indices
        if (PreferPooledBuffer(VerticesSize))
        {
            RHI::UBufferPoolHandle PooledBuffer(
                VerticesSize,
                alignof(Mdl::MeshVertex),
                RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

            PooledBuffer.AsUpload().Write(PooledBuffer.Offset, Vertices.data(), VerticesSize);

            VertexBuffer = std::move(PooledBuffer);
        }
        else
        {
            VertexBuffer = RHI::USyncGpuResource(
                RHI::ResourceDesc::Buffer(VerticesSize),
                RHI::ComputeSubresource(Vertices.data(), VerticesSize));
        }

        if (PreferPooledBuffer(IndicesSize))
        {
            RHI::UBufferPoolHandle PooledBuffer(
                IndicesSize,
                SmallIndices ? alignof(uint16_t) : alignof(uint32_t),
                RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

            if (SmallIndices)
            {
                uint16_t* IndicesPtr = PooledBuffer.AsUpload().Map<uint16_t>(PooledBuffer.Offset);
                for (auto& Index : Indices)
                {
                    *IndicesPtr++ = uint16_t(Index);
                }
                PooledBuffer.AsUpload().Unmap();
            }
            else
            {
                PooledBuffer.AsUpload().Write(PooledBuffer.Offset, Indices.data(), IndicesSize);
            }

            IndexBuffer = std::move(PooledBuffer);
        }
        else
        {
            if (SmallIndices)
            {
                std::vector<uint16_t> IndicesU16(Indices.begin(), Indices.end());
                IndexBuffer = RHI::USyncGpuResource(
                    RHI::ResourceDesc::Buffer(IndicesSize),
                    RHI::ComputeSubresource(IndicesU16.data(), IndicesSize));
            }
            else
            {
                IndexBuffer = RHI::USyncGpuResource(
                    RHI::ResourceDesc::Buffer(IndicesSize),
                    RHI::ComputeSubresource(Indices.data(), IndicesSize));
            }
        }

        return Mdl::Mesh(
            std::make_shared<Mdl::Model>(
                std::move(VertexBuffer),
                std::move(IndexBuffer),
                SmallIndices,
                std::move(Submeshes),
                std::move(Nodes),
                std::move(Materials)),
            { 0 });
    }

    //

    void Brush::Rebuild()
    {
        ModelInfo Target(m_Target);
        ModelInfo Brush_(m_Brush);

        switch (m_Operation)
        {
        case Operation::Union:
        {
            auto Final  = csg_union(&Target, &Brush_);
            m_FinalMesh = csg_to_mdl(Final, m_Target.GetMaterial());
            break;
        }
        case Operation::Subtraction:
        {
            auto Final  = csg_subtract(&Target, &Brush_);
            m_FinalMesh = csg_to_mdl(Final, m_Target.GetMaterial());
            break;
        }
        case Operation::Intersection:
        {
            auto Final  = csg_intersect(&Target, &Brush_);
            m_FinalMesh = csg_to_mdl(Final, m_Target.GetMaterial());
            break;
        }
        }
    }

    //

    CSGNode::CSGNode(const std::vector<Polygon>& list) :
        front(nullptr), back(nullptr)
    {
        build(list);
    }

    CSGNode::~CSGNode()
    {
        std::deque<CSGNode*> nodes_to_delete;
        std::deque<CSGNode*> nodes_to_disassemble;

        nodes_to_disassemble.push_back(this);
        while (nodes_to_disassemble.size())
        {
            CSGNode* me = nodes_to_disassemble.front();
            nodes_to_disassemble.pop_front();

            if (me->front)
            {
                nodes_to_disassemble.push_back(me->front);
                nodes_to_delete.push_back(me->front);
                me->front = NULL;
            }
            if (me->back)
            {
                nodes_to_disassemble.push_back(me->back);
                nodes_to_delete.push_back(me->back);
                me->back = NULL;
            }
        }

        for (auto it = nodes_to_delete.begin(); it != nodes_to_delete.end(); ++it)
            delete *it;
    }

    CSGNode* CSGNode::clone() const
    {
        CSGNode* ret = new CSGNode();

        std::deque<std::pair<const CSGNode*, CSGNode*>> nodes;
        nodes.emplace_back(this, ret);
        while (nodes.size())
        {
            const CSGNode* original = nodes.front().first;
            CSGNode*       clone    = nodes.front().second;
            nodes.pop_front();

            clone->polygons = original->polygons;
            clone->plane    = original->plane;
            if (original->front)
            {
                clone->front = new CSGNode();
                nodes.emplace_back(original->front, clone->front);
            }
            if (original->back)
            {
                clone->back = new CSGNode();
                nodes.emplace_back(original->back, clone->back);
            }
        }

        return ret;
    }

    // Remove all polygons in this BSP tree that are inside the other BSP tree
    // `bsp`.
    void CSGNode::clipto(const CSGNode* other)
    {
        std::deque<CSGNode*> nodes;
        nodes.push_back(this);
        while (nodes.size())
        {
            CSGNode* me = nodes.front();
            nodes.pop_front();

            me->polygons = other->clippolygons(me->polygons);
            if (me->front)
                nodes.push_back(me->front);
            if (me->back)
                nodes.push_back(me->back);
        }
    }

    // Convert solid space to empty space and empty space to solid space.
    void CSGNode::invert()
    {
        std::deque<CSGNode*> nodes;
        nodes.push_back(this);
        while (nodes.size())
        {
            CSGNode* me = nodes.front();
            nodes.pop_front();

            for (size_t i = 0; i < me->polygons.size(); i++)
                me->polygons[i].flip();
            me->plane.Flip();
            std::swap(me->front, me->back);
            if (me->front)
                nodes.push_back(me->front);
            if (me->back)
                nodes.push_back(me->back);
        }
    }

    // Build a BSP tree out of `polygons`. When called on an existing tree, the
    // new polygons are filtered down to the bottom of the tree and become new
    // nodes there. Each set of polygons is partitioned using the first polygon
    // (no heuristic is used to pick a good split).
    void CSGNode::build(const std::vector<Polygon>& ilist)
    {
        if (!ilist.size())
            return;

        std::deque<std::pair<CSGNode*, std::vector<Polygon>>> builds;
        builds.emplace_back(this, ilist);

        while (builds.size())
        {
            auto& [me, list] = builds.front();

            NEON_ASSERT(list.size() > 0 && "logic error");

            if (glm::length(me->plane.GetNormal()) <= 0.f)
                me->plane = list[0].plane;
            std::vector<Polygon> list_front, list_back;

            // me->polygons.push_back(list[0]);
            for (size_t i = 0; i < list.size(); i++)
                splitpolygon(me->plane, list[i], me->polygons, me->polygons, list_front, list_back);

            if (list_front.size())
            {
                if (!me->front)
                    me->front = new CSGNode;
                builds.emplace_back(me->front, std::move(list_front));
            }
            if (list_back.size())
            {
                if (!me->back)
                    me->back = new CSGNode;
                builds.emplace_back(me->back, std::move(list_back));
            }

            builds.pop_front();
        }
    }

    // Recursively remove all polygons in `polygons` that are inside this BSP
    // tree.
    auto CSGNode::clippolygons(const std::vector<Polygon>& ilist) const -> std::vector<Polygon>
    {
        std::vector<Polygon> result;

        std::deque<std::pair<const CSGNode* const, std::vector<Polygon>>> clips;
        clips.emplace_back(this, ilist);
        while (clips.size())
        {
            auto& [me, list] = clips.front();

            if (glm::length(me->plane.GetNormal()) <= 0.f)
            {
                result.insert(result.end(), list.begin(), list.end());
                clips.pop_front();
                continue;
            }

            std::vector<Polygon> list_front, list_back;
            for (size_t i = 0; i < list.size(); i++)
                splitpolygon(me->plane, list[i], list_front, list_back, list_front, list_back);

            if (me->front)
                clips.emplace_back(me->front, std::move(list_front));
            else
                result.insert(result.end(), list_front.begin(), list_front.end());

            if (me->back)
                clips.emplace_back(me->back, std::move(list_back));

            clips.pop_front();
        }

        return result;
    }

    // Return a list of all polygons in this BSP tree.
    auto CSGNode::allpolygons() const -> std::vector<Polygon>
    {
        std::vector<Polygon> result;

        std::deque<const CSGNode*> nodes;
        nodes.push_back(this);
        while (nodes.size())
        {
            const CSGNode* me = nodes.front();
            nodes.pop_front();

            result.insert(result.end(), me->polygons.begin(), me->polygons.end());
            if (me->front)
                nodes.push_back(me->front);
            if (me->back)
                nodes.push_back(me->back);
        }

        return result;
    }

    void CSGNode::splitpolygon(
        const Math::Plane& plane,
        const Polygon& poly, std::vector<Polygon>& coplanarFront,
        std::vector<Polygon>& coplanarBack, std::vector<Polygon>& front,
        std::vector<Polygon>& back)
    {
        // Classify each point as well as the entire polygon into one of the above
        // four classes.
        int polygonType = 0;
        for (const auto& v : poly.vertices)
        {
            polygonType |= classify(plane, v.Position);
        }

        // Put the polygon in the correct list, splitting it when necessary.
        switch (polygonType)
        {
        case CLS_COPLANAR:
        {
            if (glm::dot(plane.GetNormal(), poly.plane.GetNormal()) > 0)
                coplanarFront.push_back(poly);
            else
                coplanarBack.push_back(poly);
            break;
        }
        case CLS_FRONT:
        {
            front.push_back(poly);
            break;
        }
        case CLS_BACK:
        {
            back.push_back(poly);
            break;
        }
        case CLS_SPANNING:
        {
            std::vector<Mdl::MeshVertex> f, b;

            for (size_t i = 0; i < poly.vertices.size(); i++)
            {
                size_t j = (i + 1) % poly.vertices.size();

                const auto& vi = poly.vertices[i];
                const auto& vj = poly.vertices[j];

                int ti = classify(plane, vi.Position);
                int tj = classify(plane, vj.Position);

                if (ti != CLS_BACK)
                    f.push_back(vi);
                if (ti != CLS_FRONT)
                    b.push_back(vi);
                if ((ti | tj) == CLS_SPANNING)
                {
                    float           t    = (plane.w - glm::dot(plane.GetNormal(), vi.Position)) / glm::dot(plane.GetNormal(), vj.Position - vi.Position);
                    Mdl::MeshVertex copy = vi;
                    copy.Position        = vi.Position + (vj.Position - vi.Position) * t;
                    copy.Normal          = glm::normalize(vi.Normal + (vj.Normal - vi.Normal) * t);
                    copy.Tangent         = glm::normalize(vi.Tangent + (vj.Tangent - vi.Tangent) * t);
                    copy.Bitangent       = glm::normalize(vi.Bitangent + (vj.Bitangent - vi.Bitangent) * t);
                    copy.TexCoord        = vi.TexCoord + (vj.TexCoord - vi.TexCoord) * t;
                    f.push_back(copy);
                    b.push_back(copy);
                }
            }

            if (f.size() >= 3)
                front.emplace_back(std::move(f));
            if (b.size() >= 3)
                back.emplace_back(std::move(b));
            break;
        }
        }
    }
} // namespace Neon::Scene::CSG