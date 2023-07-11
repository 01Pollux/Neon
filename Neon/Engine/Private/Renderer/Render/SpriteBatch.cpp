#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>

#include <Resource/Pack.hpp>
#include <Resource/Types/RootSignature.hpp>
#include <Resource/Types/Shader.hpp>

#include <RHI/Swapchain.hpp>
#include <RHI/GlobalDescriptors.hpp>
#include <RHI/Resource/State.hpp>
#include <RHI/Resource/Views/Shader.hpp>

namespace Neon::Renderer
{
    namespace SpriteBatchConstants
    {
        constexpr uint32_t VerticesCount = 4;
        constexpr uint32_t IndicesCount  = 6;

        constexpr uint32_t MaxPoints   = 10'000;
        constexpr uint32_t MaxVertices = MaxPoints * VerticesCount;
        constexpr uint32_t MaxIndices  = MaxPoints * IndicesCount;

        using IndexType = uint16_t;
        static_assert(MaxIndices < std::numeric_limits<IndexType>::max());

        struct Layout
        {
            Vector3 Position;
            Vector2 TexCoord;
            int32_t MaterialIndex;
            Color4  Color;
        };

        /// <summary>
        /// a sprite's quad is made up of 4 vertices and looks like this:
        /// 0---1
        /// |   |
        /// 3---2
        /// </summary>
        void CreateIndexInstance(
            IndexType* IndexBuffer,
            IndexType& Index,
            IndexType& Offset)
        {
            IndexBuffer[Index + 0] = Offset;
            IndexBuffer[Index + 1] = Offset + 1;
            IndexBuffer[Index + 2] = Offset + 2;

            IndexBuffer[Index + 3] = Offset;
            IndexBuffer[Index + 4] = Offset + 2;
            IndexBuffer[Index + 5] = Offset + 3;

            Index += IndicesCount;
            Offset += VerticesCount;
        }
    } // namespace SpriteBatchConstants

    SpriteBatch::SpriteBatch()
    {
        CreateBuffers();
    }

    //

    void SpriteBatch::Begin(
        RHI::IGraphicsCommandList* CommandList)
    {
        m_CommandList = CommandList;

        m_DrawCount     = 0;
        m_VerticesCount = 0;

        m_PipelineState = nullptr;
    }

    void SpriteBatch::Draw(
        const Scene::Component::Transform& Transform,
        const Scene::Component::Sprite&    Sprite)
    {
        if (!Sprite.MaterialInstance)
        {
            return;
        }

        auto PipelineState = Sprite.MaterialInstance->GetPipelineState().get();
        // This is not our first time calling ::Draw()
        if (m_PipelineState) [[likely]]
        {
            if ((m_PipelineState != PipelineState) || (m_DrawCount >= SpriteBatchConstants::MaxPoints)) [[unlikely]]
            {
                auto CommandList = m_CommandList;
                End();
                Begin(CommandList);
            }
            m_PipelineState = PipelineState;
        }
        else
        {
            m_PipelineState = PipelineState;
        }

        Vector2 TexCoords[]{
            Sprite.TextureRect.TopLeft(),
            Sprite.TextureRect.TopRight(),
            Sprite.TextureRect.BottomRight(),
            Sprite.TextureRect.BottomLeft()
        };

        Vector2 HalfSize = Sprite.Size * 0.5f;
        Vector3 QuadPositions[]{
            Transform.World.GetPosition() + Vector3(-HalfSize.x, HalfSize.y, 0.f),
            Transform.World.GetPosition() + Vector3(HalfSize.x, HalfSize.y, 0.f),
            Transform.World.GetPosition() + Vector3(HalfSize.x, -HalfSize.y, 0.f),
            Transform.World.GetPosition() + Vector3(-HalfSize.x, -HalfSize.y, 0.f)
        };

        int MaterialIndex = m_MaterialTable.Append(Sprite.MaterialInstance.get());

        // Write vertices to the vertex buffer
        for (size_t i = 0; i < SpriteBatchConstants::VerticesCount; ++i, ++m_VerticesCount)
        {
            auto Buffer = std::bit_cast<SpriteBatchConstants::Layout*>(m_VertexBufferPtr) + m_VerticesCount;

            Buffer->Position      = QuadPositions[i];
            Buffer->TexCoord      = TexCoords[i];
            Buffer->Color         = Sprite.ModulationColor;
            Buffer->MaterialIndex = MaterialIndex;
        }

        m_DrawCount++;
    }

    void SpriteBatch::End()
    {
        if (m_DrawCount == 0)
        {
            return;
        }

        //

        MaterialBinder MatBinder(m_MaterialTable.GetMaterials());
        MatBinder.BindAll(m_CommandList);

        RHI::Views::Vertex VertexView;
        VertexView.Append(
            m_VertexBuffer->GetHandle(),
            sizeof(SpriteBatchConstants::Layout),
            m_VertexBuffer->GetSize());

        RHI::Views::Index IndexView(
            m_IndexBuffer->GetHandle(),
            m_IndexBuffer->GetSize());

        m_CommandList->SetVertexBuffer(0, VertexView);
        m_CommandList->SetIndexBuffer(IndexView);
        m_CommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

        m_CommandList->Draw(
            RHI::DrawIndexArgs{
                .IndexCountPerInstance = m_DrawCount * SpriteBatchConstants::IndicesCount });

        m_CommandList = nullptr;
    }

    //

    void SpriteBatch::CreateBuffers()
    {
        m_VertexBuffer.reset(RHI::IUploadBuffer::Create(
            {
                .Size = sizeof(SpriteBatchConstants::Layout) * SpriteBatchConstants::MaxVertices,
            }));

        m_VertexBufferPtr = m_VertexBuffer->Map();

        m_IndexBuffer.reset(RHI::IUploadBuffer::Create(
            {
                .Size = sizeof(SpriteBatchConstants::IndexType) * SpriteBatchConstants::MaxIndices,
            }));

        auto IndexBuffer = m_IndexBuffer->Map<SpriteBatchConstants::IndexType>();

        SpriteBatchConstants::IndexType Offset = 0, Index = 0;
        while (Index < SpriteBatchConstants::MaxIndices)
        {
            SpriteBatchConstants::CreateIndexInstance(IndexBuffer, Index, Offset);
        }

        m_IndexBuffer->Unmap();
    }
} // namespace Neon::Renderer