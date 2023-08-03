#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

#include <Scene/Component/Transform.hpp>
#include <Scene/Component/Sprite.hpp>

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

        constexpr uint32_t MaxPoints   = 500;
        constexpr uint32_t MaxVertices = MaxPoints * VerticesCount;
        constexpr uint32_t MaxIndices  = MaxPoints * IndicesCount;

        using IndexType = uint16_t;
        static_assert(MaxIndices < std::numeric_limits<IndexType>::max());

        struct PerObjectData
        {
            Matrix4x4 World;
            Vector4   Color;
        };

        struct VertexInput
        {
            Vector2 Position;
            Vector2 TexCoord;
            int     SpriteIndex;
        };

        static constexpr size_t VertexSize        = sizeof(VertexInput);
        static constexpr size_t PerObjectDataSize = sizeof(PerObjectData);

        static constexpr size_t IndexBufferSize         = MaxIndices * sizeof(IndexType);
        static constexpr size_t VertexBufferSize        = (MaxVertices * VertexSize);
        static constexpr size_t PerObjectDataBufferSize = (MaxPoints * PerObjectDataSize);

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

    void SpriteBatch::SetCameraBuffer(
        const Ptr<RHI::IUploadBuffer>& Buffer)
    {
        m_CameraBuffer = Buffer;
    }

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
        Vector2 QuadPositions[]{
            Vector2(-HalfSize.x, HalfSize.y),
            Vector2(HalfSize.x, HalfSize.y),
            Vector2(HalfSize.x, -HalfSize.y),
            Vector2(-HalfSize.x, -HalfSize.y)
        };

        m_MaterialInstances.Append(Sprite.MaterialInstance.get());

        // Write vertices to the vertex buffer
        for (size_t i = 0; i < SpriteBatchConstants::VerticesCount; ++i, ++m_VerticesCount)
        {
            auto Buffer = std::bit_cast<SpriteBatchConstants::VertexInput*>(m_VertexBufferPtr) + m_VerticesCount;

            Buffer->Position    = QuadPositions[i];
            Buffer->TexCoord    = TexCoords[i];
            Buffer->SpriteIndex = m_DrawCount;
        }

        auto PerObjectDataBuffer = std::bit_cast<SpriteBatchConstants::PerObjectData*>(m_PerDataBufferPtr) + m_DrawCount;

        PerObjectDataBuffer->World = glm::transpose(Transform.World.ToMat4x4());
        PerObjectDataBuffer->Color = Sprite.ModulationColor;

        m_DrawCount++;
    }

    void SpriteBatch::End()
    {
        if (m_DrawCount == 0)
        {
            return;
        }

        auto FirstMaterial = m_MaterialInstances.GetMaterial(0);

        FirstMaterial->SetStructuredBuffer(
            "g_SpriteData",
            m_PerDataBuffer,
            RHI::SRVDesc{
                .View = RHI::SRVDesc::Buffer{
                    .Count        = m_DrawCount,
                    .SizeOfStruct = SpriteBatchConstants::PerObjectDataSize } });

        FirstMaterial->SetResourceView(
            "g_FrameData",
            m_CameraBuffer->GetHandle());

        m_MaterialInstances.Bind(m_CommandList);

        RHI::Views::Vertex VertexView;
        VertexView.Append(
            m_VertexBuffer->GetHandle(),
            sizeof(SpriteBatchConstants::VertexInput),
            m_DrawCount * SpriteBatchConstants::VerticesCount * sizeof(SpriteBatchConstants::VertexInput));

        RHI::Views::Index IndexView(
            m_IndexBuffer->GetHandle(),
            m_DrawCount * SpriteBatchConstants::IndicesCount * sizeof(SpriteBatchConstants::IndexType));

        m_CommandList->SetVertexBuffer(0, VertexView);
        m_CommandList->SetIndexBuffer(IndexView);
        m_CommandList->SetPrimitiveTopology(RHI::PrimitiveTopology::TriangleList);

        m_CommandList->Draw(
            RHI::DrawIndexArgs{
                .IndexCountPerInstance = m_DrawCount * SpriteBatchConstants::IndicesCount });

        m_CommandList = nullptr;
        m_MaterialInstances.Reset();
    }

    //

    void SpriteBatch::CreateBuffers()
    {
        // Create vertex buffer
        m_VertexBuffer.reset(RHI::IUploadBuffer::Create(
            { .Size = SpriteBatchConstants::VertexBufferSize }));
        m_VertexBufferPtr = m_VertexBuffer->Map();

        //

        // Create per-object data buffer
        m_PerDataBuffer.reset(RHI::IUploadBuffer::Create(
            { .Size = SpriteBatchConstants::PerObjectDataBufferSize, .UsePool = false }));
        m_PerDataBufferPtr = m_PerDataBuffer->Map();
        //

        // Create index buffer
        m_IndexBuffer.reset(RHI::IUploadBuffer::Create(
            {
                .Size = sizeof(SpriteBatchConstants::IndexType) * SpriteBatchConstants::MaxIndices,
            }));

        // Write indices to the index buffer
        auto IndexBuffer = m_IndexBuffer->Map<SpriteBatchConstants::IndexType>();

        SpriteBatchConstants::IndexType Offset = 0, Index = 0;
        while (Index < SpriteBatchConstants::MaxIndices)
        {
            SpriteBatchConstants::CreateIndexInstance(IndexBuffer, Index, Offset);
        }

        m_IndexBuffer->Unmap();
    }

    //

    void SpriteBatcher::SetCameraBuffer(
        const Ptr<RHI::IUploadBuffer>& Buffer)
    {
        m_CameraBuffer = Buffer;
    }

    void SpriteBatcher::OnDraw()
    {
        BatchBaseClass::OnDraw();

        auto FirstMaterial = m_MaterialInstances.GetMaterial(0);

        FirstMaterial->SetResourceView(
            "g_SpriteData",
            m_PerObjectBuffer.GetHandleFor(uint32_t(m_MaterialInstances.GetMaterialCount() * sizeof(PerObjectData))));

        FirstMaterial->SetResourceView(
            "g_FrameData",
            m_CameraBuffer->GetHandle());

        m_MaterialInstances.Bind(m_CommandList);
    }

    void SpriteBatcher::OnReset()
    {
        m_PerObjectBuffer.Reset();
        m_MaterialInstances.Reset();
    }

    void SpriteBatcher::Draw(
        const Scene::Component::Transform& Transform,
        const Scene::Component::Sprite&    Sprite)
    {
        if (!Sprite.MaterialInstance)
        {
            return;
        }

        if (!m_PerObjectBuffer.Reserve(sizeof(PerObjectData)))
        {
            Reset();
        }

        Vector2    HalfSize = Sprite.Size * 0.5f;
        std::array Vertices = {
            BatchSpriteVertex{ .Position = Vector2(-HalfSize.x, HalfSize.y), .TexCoord = Sprite.TextureRect.TopLeft(), .SpriteIndex = m_MaterialInstances.GetMaterialCount() },
            BatchSpriteVertex{ .Position = Vector2(HalfSize.x, HalfSize.y), .TexCoord = Sprite.TextureRect.TopRight(), .SpriteIndex = m_MaterialInstances.GetMaterialCount() },
            BatchSpriteVertex{ .Position = Vector2(HalfSize.x, -HalfSize.y), .TexCoord = Sprite.TextureRect.BottomRight(), .SpriteIndex = m_MaterialInstances.GetMaterialCount() },
            BatchSpriteVertex{ .Position = Vector2(-HalfSize.x, -HalfSize.y), .TexCoord = Sprite.TextureRect.BottomLeft(), .SpriteIndex = m_MaterialInstances.GetMaterialCount() }
        };

        m_MaterialInstances.Append(Sprite.MaterialInstance.get());
        DrawQuad<>(Vertices);
    }
} // namespace Neon::Renderer