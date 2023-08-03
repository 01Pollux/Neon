#pragma once

#include <Renderer/Material/Material.hpp>
#include <RHI/Resource/Resource.hpp>
#include <RHI/Commands/List.hpp>

#include <Math/Vector.hpp>
#include <Math/Rect.hpp>
#include <Math/Colors.hpp>

namespace Neon::Renderer
{
    namespace Impl
    {
        class PrimitiveBatch
        {
        protected:
            PrimitiveBatch(
                uint32_t VertexStride,
                uint32_t VetexCount,
                uint32_t IndexStride,
                uint32_t IndexCount);

            /// <summary>
            /// Begin drawing.
            /// </summary>
            void Begin(
                RHI::IGraphicsCommandList* CommandList,
                RHI::PrimitiveTopology     Topology,
                bool                       Indexed = false);

            /// <summary>
            /// Draw a primitive.
            /// </summary>
            void Draw(
                uint32_t VertexSize,
                void**   OutVertices,
                uint32_t IndexSize,
                void**   OutIndices);

            /// <summary>
            /// End drawing.
            /// </summary>
            void End();

        private:
            RHI::IGraphicsCommandList* m_CommandList = nullptr;

            /// <summary>
            /// Stride of the vertex buffer.
            /// </summary>
            uint32_t m_VertexStride;
            /// <summary>
            /// Number of vertices in the vertex buffer.
            /// </summary>
            uint32_t m_VertexCount = 0;
            /// <summary>
            /// Offset of the vertex buffer.
            /// </summary>
            uint32_t m_VertexOffset = 0;
            /// <summary>
            /// Maximum number of vertices in the vertex buffer.
            /// </summary>
            uint32_t m_VertexMaxCount;

            /// <summary>
            /// Stride of the index buffer.
            /// </summary>
            uint32_t m_IndexCount = 0;
            /// <summary>
            /// Offset of the index buffer.
            /// </summary>
            uint32_t m_IndexOffset = 0;
            /// <summary>
            /// Maximum number of indices in the index buffer.
            /// </summary>
            uint32_t m_IndexMaxCount;

            UPtr<RHI::IUploadBuffer> m_VertexBuffer;
            UPtr<RHI::IUploadBuffer> m_IndexBuffer;

            void* m_MappedVertices = nullptr;
            void* m_MappedIndices  = nullptr;

            RHI::PrimitiveTopology m_Topology;

            bool m_DrawingIndexed : 1 = false;
            bool m_Is32BitIndex   : 1 = false;
        };
    } // namespace Impl

    /// <summary>
    /// Primitive batch.
    /// _IdxType can be uint16_t or uint32_t or void.
    /// </summary>
    template<typename _VertexTy, typename _IdxType = uint16_t>
    class PrimitiveBatch : public Impl::PrimitiveBatch
    {
    public:
        static constexpr bool UsingIndexBuffer = !std::is_same_v<_IdxType, void>;

        static constexpr size_t VertexStride = sizeof(_VertexTy);
        static constexpr size_t IndexStride  = UsingIndexBuffer ? sizeof(_IdxType) : 1;

        PrimitiveBatch(
            uint32_t VerticesCount = 4096,
            uint32_t IndexCount    = 4096 * 3) :
            Impl::PrimitiveBatch(
                VerticesCount * VertexStride,
                UsingIndexBuffer ? IndexCount * IndexStride : 0)
        {
        }

        /// <summary>
        /// Begin drawing.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void Begin(
            RHI::IGraphicsCommandList* CommandList,
            RHI::PrimitiveTopology     Topology)
        {
            return Impl::PrimitiveBatch::Begin(
                CommandList,
                Topology,
                _ForceIndexed);
        }

        /// <summary>
        /// End drawing.
        /// </summary>
        void End()
        {
            Impl::PrimitiveBatch::End();
        }

        /// <summary>
        /// Draw a primitive.
        /// </summary>
        void Draw(
            RHI::PrimitiveTopology Topology,
            const _VertexTy*       Vertices,
            uint32_t               VertexCount)
        {
            _VertexTy* OutVertices = nullptr;
            Impl::PrimitiveBatch::Draw(
                VertexCount * VertexStride,
                &OutVertices,
                0,
                nullptr);
            std::copy_n(Vertices, VertexCount, OutVertices);
        }

        /// <summary>
        /// Draw indexed primitive.
        /// </summary>
        void DrawIndexed(
            RHI::PrimitiveTopology Topology,
            const _VertexTy*       Vertices,
            uint32_t               VertexCount,
            const _IdxType*        Indices,
            uint32_t               IndexCount)
        {
            _VertexTy* OutVertices = nullptr;
            _IdxType*  OutIndices  = nullptr;
            Impl::PrimitiveBatch::Draw(
                VertexCount * VertexStride,
                &OutVertices,
                IndexCount * IndexStride,
                &OutIndices);
            std::copy_n(Vertices, VertexCount, OutVertices);
            std::copy_n(Indices, IndexCount, OutIndices);
        }

    public:
        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void DrawLine(
            const _VertexTy& V0,
            const _VertexTy& V1)
        {
            _VertexTy* Vertices = nullptr;
            _IdxType*  Indices  = nullptr;
            _IdxType** PtrIndices;

            if constexpr (_ForceIndexed)
            {
                PtrIndices = &Indices;
            }
            else
            {
                PtrIndices = nullptr;
            }

            Impl::PrimitiveBatch::Draw(
                2,
                &Vertices,
                2,
                PtrIndices);

            Vertices[0] = V0;
            Vertices[1] = V1;

            if constexpr (_ForceIndexed)
            {
                Indices[0] = 0;
                Indices[1] = 1;
            }
        }

        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void DrawTriangle(
            const _VertexTy& V0,
            const _VertexTy& V1,
            const _VertexTy& V2)
        {
            _VertexTy* Vertices = nullptr;
            _IdxType*  Indices  = nullptr;
            _IdxType** PtrIndices;

            if constexpr (_ForceIndexed)
            {
                PtrIndices = &Indices;
            }
            else
            {
                PtrIndices = nullptr;
            }

            Impl::PrimitiveBatch::Draw(
                3,
                &Vertices,
                3,
                PtrIndices);

            Vertices[0] = V0;
            Vertices[1] = V1;
            Vertices[2] = V2;

            if constexpr (_ForceIndexed)
            {
                Indices[0] = 0;
                Indices[1] = 1;
                Indices[2] = 2;
            }
        }

        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void DrawQuad(
            const _VertexTy& V0,
            const _VertexTy& V1,
            const _VertexTy& V2,
            const _VertexTy& V3)
        {
            _VertexTy* Vertices = nullptr;
            _IdxType*  Indices  = nullptr;
            _IdxType** PtrIndices;

            if constexpr (_ForceIndexed)
            {
                PtrIndices = &Indices;
            }
            else
            {
                PtrIndices = nullptr;
            }

            Impl::PrimitiveBatch::Draw(
                4,
                &Vertices,
                6,
                PtrIndices);

            Vertices[0] = V0;
            Vertices[1] = V1;
            Vertices[2] = V2;
            Vertices[3] = V3;

            if constexpr (_ForceIndexed)
            {
                Indices[0] = 0;
                Indices[1] = 1;
                Indices[2] = 2;
                Indices[3] = 0;
                Indices[4] = 2;
                Indices[5] = 3;
            }
        }
    };

    /// <summary>
    /// Primitive batch for uint16_t
    /// </summary>
    template<typename _VertexTy>
    using PrimitiveBatch16 = PrimitiveBatch<_VertexTy, uint16_t>;

    /// <summary>
    /// Primitive batch for uint32_t
    /// </summary>
    template<typename _VertexTy>
    using PrimitiveBatch32 = PrimitiveBatch<_VertexTy, uint32_t>;

    /// <summary>
    /// Primitive batch for void (no index buffer)
    /// </summary>
    template<typename _VertexTy>
    using PrimitiveBatchVoid = PrimitiveBatch<_VertexTy, void>;
} // namespace Neon::Renderer