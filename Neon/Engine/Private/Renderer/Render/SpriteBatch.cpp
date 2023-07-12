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

        constexpr uint32_t MaxPoints   = 500;
        constexpr uint32_t MaxVertices = MaxPoints * VerticesCount;
        constexpr uint32_t MaxIndices  = MaxPoints * IndicesCount;

        using IndexType = uint16_t;
        static_assert(MaxIndices < std::numeric_limits<IndexType>::max());

        struct PerObjectData
        {
            alignas(16) Matrix4x4 World;
            alignas(16) Vector4 Color;
            alignas(16) int TextureIndex;
        };

        struct VertexInput
        {
            Vector3 Position;
            Vector2 TexCoord;
            int     SpriteIndex;
        };

        static constexpr size_t VertexSize        = sizeof(VertexInput);
        static constexpr size_t PerObjectDataSize = sizeof(PerObjectData);

        static constexpr size_t IndexBufferSize         = MaxIndices * sizeof(IndexType);
        static constexpr size_t VertexBufferSize        = Math::AlignUp(MaxVertices * VertexSize, 256);
        static constexpr size_t PerObjectDataBufferSize = Math::AlignUp(MaxPoints * PerObjectDataSize, 256);

        static constexpr size_t VertexBufferOffset        = 0;
        static constexpr size_t PerObjectDataBufferOffset = VertexBufferSize;

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
            auto Buffer = std::bit_cast<SpriteBatchConstants::VertexInput*>(GetVertexBuffer()) + m_VerticesCount;

            Buffer->Position    = QuadPositions[i];
            Buffer->TexCoord    = TexCoords[i];
            Buffer->SpriteIndex = m_DrawCount;
        }

        auto PerObjectDataBuffer = std::bit_cast<SpriteBatchConstants::PerObjectData*>(GetPerObjectBuffer()) + m_DrawCount;

        PerObjectDataBuffer->World        = glm::transpose(Transform.World.ToMat4x4());
        PerObjectDataBuffer->Color        = Sprite.ModulationColor;
        PerObjectDataBuffer->TextureIndex = MaterialIndex;

        m_DrawCount++;
    }

    void SpriteBatch::End()
    {
        if (m_DrawCount == 0)
        {
            return;
        }

        auto           FirstMaterial = m_MaterialTable.GetMaterial(0);
        MaterialBinder MatBinder(m_MaterialTable.GetMaterials());

        FirstMaterial->SetConstantBuffer(
            "g_SpriteData",
            m_VertexAndPerDataBuffer,
            RHI::CBVDesc{
                .Resource = m_VertexAndPerDataBuffer->GetHandle(SpriteBatchConstants::PerObjectDataBufferOffset),
                .Size     = SpriteBatchConstants::PerObjectDataBufferSize });

        FirstMaterial->SetConstantBuffer(
            "g_FrameData",
            m_CameraBuffer,
            RHI::CBVDesc{
                .Resource = m_CameraBuffer->GetHandle(),
                .Size     = m_CameraBuffer->GetSize() });

        MatBinder.BindAll(m_CommandList);

        RHI::Views::Vertex VertexView;
        VertexView.Append(
            m_VertexAndPerDataBuffer->GetHandle(),
            sizeof(SpriteBatchConstants::VertexInput),
            m_VertexAndPerDataBuffer->GetSize());

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
        // Create vertex and per-object data buffer
        m_VertexAndPerDataBuffer.reset(RHI::IUploadBuffer::Create(
            {
                .Size = SpriteBatchConstants::VertexBufferSize +
                        SpriteBatchConstants::PerObjectDataBufferSize,
            }));
        m_VertexBufferPtr = m_VertexAndPerDataBuffer->Map();

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

    uint8_t* SpriteBatch::GetVertexBuffer() noexcept
    {
        return m_VertexBufferPtr + SpriteBatchConstants::VertexBufferOffset;
    }

    uint8_t* SpriteBatch::GetPerObjectBuffer() noexcept
    {
        return m_VertexBufferPtr + SpriteBatchConstants::PerObjectDataBufferOffset;
    }
} // namespace Neon::Renderer