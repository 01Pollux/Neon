#pragma once

#include <Renderer/Render/FrameBuffer.hpp>
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
        public:
            NEON_CLASS_NO_COPYMOVE(PrimitiveBatch);
            virtual ~PrimitiveBatch() = default;

            /// <summary>
            /// Called before drawing.
            /// </summary>
            virtual void OnBegin()
            {
            }

            /// <summary>
            /// Called before final drawing.
            /// </summary>
            virtual void OnDraw()
            {
            }

            /// <summary>
            /// Called after drawing.
            /// </summary>
            virtual void OnReset()
            {
            }

            virtual void OnEnd()
            {
            }

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
                RHI::ICommandList* CommandList,
                RHI::PrimitiveTopology     Topology,
                bool                       Indexed = false);

            /// <summary>
            /// Draw a primitive.
            /// Returns index of the first vertex to draw to.
            /// </summary>
            [[nodiscard]] uint32_t Draw(
                uint32_t VertexSize,
                void**   OutVertices,
                uint32_t IndexSize,
                void**   OutIndices);

            /// <summary>
            /// End drawing.
            /// </summary>
            void End();

            /// <summary>
            /// Reset the batch.
            /// </summary>
            void Reset();

        protected:
            RHI::ICommandList* m_CommandList = nullptr;

        private:
            /// <summary>
            /// Stride of the vertex buffer.
            /// </summary>
            uint32_t m_VertexStride;

            FrameBuffer m_VertexBuffer;
            FrameBuffer m_IndexBuffer;

            uint32_t m_VerticesSize = 0;
            uint32_t m_IndicesSize  = 0;

            RHI::PrimitiveTopology m_Topology = RHI::PrimitiveTopology::Undefined;

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

        static constexpr size_t MaxVerticesCount = 4096;

        PrimitiveBatch(
            uint32_t VerticesCount = MaxVerticesCount,
            uint32_t IndicesCount  = MaxVerticesCount * 4) :
            Impl::PrimitiveBatch(
                VertexStride,
                VerticesCount,
                IndexStride,
                IndicesCount)
        {
        }

        /// <summary>
        /// Begin drawing.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void Begin(
            RHI::ICommandList* CommandList,
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

    public:
        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void DrawLine(
            const _VertexTy& V0,
            const _VertexTy& V1)
        {
            DrawLine<_ForceIndexed>({ V0, V1 });
        }

        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void DrawLine(
            std::span<const _VertexTy, 2> Line)
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

            uint32_t VtxIndex = Impl::PrimitiveBatch::Draw(
                VertexStride * 2,
                std::bit_cast<void**>(&Vertices),
                IndexStride * 2,
                std::bit_cast<void**>(PtrIndices));

            std::copy_n(Line.data(), 2, Vertices);

            if constexpr (_ForceIndexed)
            {
                Indices[0] = VtxIndex + 0;
                Indices[1] = VtxIndex + 1;
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
            DrawTriangle<_ForceIndexed>({ V0, V1, V2 });
        }

        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void DrawTriangle(
            std::span<const _VertexTy, 3> Tri)
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

            uint32_t VtxIndex = Impl::PrimitiveBatch::Draw(
                VertexStride * 3,
                std::bit_cast<void**>(&Vertices),
                IndexStride * 3,
                std::bit_cast<void**>(PtrIndices));

            std::copy_n(Tri.begin(), 3, Vertices);

            if constexpr (_ForceIndexed)
            {
                Indices[0] = VtxIndex + 0;
                Indices[1] = VtxIndex + 1;
                Indices[2] = VtxIndex + 2;
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
            DrawQuad<_ForceIndexed>({ V0, V1, V2, V3 });
        }

        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        template<bool _ForceIndexed = UsingIndexBuffer>
        void DrawQuad(
            std::span<const _VertexTy, 4> Quad)
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

            uint32_t VtxIndex = Impl::PrimitiveBatch::Draw(
                VertexStride * 4,
                std::bit_cast<void**>(&Vertices),
                IndexStride * 6,
                std::bit_cast<void**>(PtrIndices));

            std::copy_n(Quad.data(), 4, Vertices);

            if constexpr (_ForceIndexed)
            {
                Indices[0] = VtxIndex + 0;
                Indices[1] = VtxIndex + 1;
                Indices[2] = VtxIndex + 2;
                Indices[3] = VtxIndex + 0;
                Indices[4] = VtxIndex + 2;
                Indices[5] = VtxIndex + 3;
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