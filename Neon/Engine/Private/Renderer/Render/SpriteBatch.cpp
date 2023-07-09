#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteBatch.hpp>

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
        m_MaterialInstances.clear();
    }

    void SpriteBatch::Draw(
        const QuadCommand& Quad)
    {
        if (m_DrawCount >= SpriteBatchConstants::MaxPoints)
        {
            auto CommandList = m_CommandList;
            End();
            Begin(CommandList);
        }

        Vector2 TexCoords[]{
            Quad.TexCoord.TopLeft(),
            Quad.TexCoord.TopRight(),
            Quad.TexCoord.BottomRight(),
            Quad.TexCoord.BottomLeft()
        };

        Vector2 HalfSize = Quad.Size * 0.5f;
        Vector3 QuadPositions[]{
            Quad.Position + Vector3(-HalfSize.x, HalfSize.y, 0.f),
            Quad.Position + Vector3(HalfSize.x, HalfSize.y, 0.f),
            Quad.Position + Vector3(HalfSize.x, -HalfSize.y, 0.f),
            Quad.Position + Vector3(-HalfSize.x, -HalfSize.y, 0.f)
        };

        int MaterialIndex = -1;
        if (Quad.MaterialInstance)
        {
            for (MaterialIndex = 0; MaterialIndex < int(m_MaterialInstances.size()); MaterialIndex++)
            {
                if (m_MaterialInstances[MaterialIndex] == Quad.MaterialInstance)
                {
                    break;
                }
            }
            if (MaterialIndex == int(m_MaterialInstances.size()))
            {
                m_MaterialInstances.push_back(Quad.MaterialInstance);
            }
        }

        for (size_t i = 0; i < SpriteBatchConstants::VerticesCount; ++i, ++m_VerticesCount)
        {
            auto Buffer = std::bit_cast<SpriteBatchConstants::Layout*>(m_VertexBufferPtr) + m_VerticesCount;

            Buffer->Position      = QuadPositions[i];
            Buffer->TexCoord      = TexCoords[i];
            Buffer->Color         = Quad.Color;
            Buffer->MaterialIndex = MaterialIndex;
        }

        m_DrawCount++;

        //
    }

    void SpriteBatch::End()
    {
        if (m_DrawCount == 0)
        {
            return;
        }

        //

        // TODO: bind shared descriptor
        auto MasterMaterial = m_MaterialInstances[0]->GetParentMaterial();
        MasterMaterial->Bind(m_CommandList);

        auto ResourceTable = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView),
             SamplerTable  = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::Sampler);

        uint32_t GPUResourceDescriptorSize = 0,
                 GPUSamplerDescriptorSize  = 0;

        std::vector<RHI::IDescriptorHeap::CopyInfo> ResourceDescriptors, SamplerDescriptors;

        RHI::DescriptorHeapHandle ResourceDescriptor, SamplerDescriptor;
        for (auto Instance : m_MaterialInstances)
        {
            Instance->GetDescriptor(&ResourceDescriptor, &SamplerDescriptor);

            if (ResourceDescriptor.Size)
            {
                ResourceDescriptors.emplace_back(ResourceDescriptor.GetCpuHandle(), ResourceDescriptor.Size);
                GPUResourceDescriptorSize += ResourceDescriptor.Size;
            }
            if (SamplerDescriptor.Size)
            {
                SamplerDescriptors.emplace_back(SamplerDescriptor.GetCpuHandle(), SamplerDescriptor.Size);
                GPUSamplerDescriptorSize += SamplerDescriptor.Size;
            }
        }

        if (GPUResourceDescriptorSize)
        {
            auto ResourceHandle = ResourceTable->Allocate(GPUResourceDescriptorSize);

            RHI::IDescriptorHeap::CopyInfo Destination{
                .Descriptor = ResourceHandle.GetCpuHandle(),
                .CopySize   = ResourceHandle.Size
            };
            RHI::IDescriptorHeap::Copy(RHI::DescriptorType::ResourceView, ResourceDescriptors, { &Destination, 1 });

            m_CommandList->SetDescriptorTable(0, ResourceHandle.GetGpuHandle());
        }
        // auto SamplerHandle = SamplerTable->Allocate(GPUSamplerDescriptorSize);

        //

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