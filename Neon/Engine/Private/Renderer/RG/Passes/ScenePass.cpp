#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

#include <Scene/Scene.hpp>
#include <Scene/Component/Sprite.hpp>
#include <Scene/Component/Transform.hpp>

//

#include <RHI/Resource/Resource.hpp>
#include <RHI/Commands/List.hpp>

//

namespace Neon::RG
{
    struct VSInput
    {
        Vector4D Position;
        Vector2D TexCoord;
    };

    using namespace Scene;

    ScenePass::ScenePass(
        GameScene& Scene) :
        IRenderPass(PassQueueType::Direct),
        m_Scene(Scene)
    {
        m_SpriteQuery = m_Scene->query_builder<
                                   Component::Transform,
                                   Component::Sprite>()
                            .build();
    }

    void ScenePass::ResolveShaders(
        ShaderResolver& Resolver)
    {
    }

    void ScenePass::ResolveRootSignature(
        RootSignatureResolver& Resolver)
    {
    }

    void ScenePass::ResolvePipelineStates(
        PipelineStateResolver& Resolver)
    {
    }

    void ScenePass::ResolveResources(
        ResourceResolver& Resolver)
    {
        Resolver.CreateBuffer(
            RG::ResourceId(STR("SpriteVertexBuffer")),
            RHI::BufferDesc{
                .Size = 1024 * 4 * sizeof(VSInput) },
            RHI::GraphicsBufferType::Upload);

        Resolver.CreateBuffer(
            RG::ResourceId(STR("SpriteIndexBuffer")),
            RHI::BufferDesc{
                .Size = 1024 * 4 * sizeof(uint16_t) },
            RHI::GraphicsBufferType::Upload);
    }

    void ScenePass::Dispatch(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        auto RenderCommandList = dynamic_cast<RHI::IGraphicsCommandList*>(CommandList);

        //

        auto VertexBuffer = Storage.GetResource(RG::ResourceId(STR("SpriteVertexBuffer"))).AsUploadBuffer();
        auto IndexBuffer  = Storage.GetResource(RG::ResourceId(STR("SpriteIndexBuffer"))).AsUploadBuffer();

        size_t DrawIndex = 0;

        auto DrawToBuffer =
            [VertexBuffer, IndexBuffer, DrawIndex](
                const Component::Transform& Transform,
                const Component::Sprite&    Sprite) mutable
        {
            auto World = Transform.World.ToMat4x4();

            //

            auto Vertex = VertexBuffer->Map<VSInput>(DrawIndex * sizeof(VSInput));

            Vertex[0].Position = World.DoTransform(Vector2D(Sprite.Scale.x() * -0.5f, Sprite.Scale.y() * +0.5f));
            Vertex[1].Position = World.DoTransform(Vector2D(Sprite.Scale.x() * +0.5f, Sprite.Scale.y() * +0.5f));
            Vertex[2].Position = World.DoTransform(Vector2D(Sprite.Scale.x() * +0.5f, Sprite.Scale.y() * -0.5f));
            Vertex[3].Position = World.DoTransform(Vector2D(Sprite.Scale.x() * -0.5f, Sprite.Scale.y() * -0.5f));

            Vertex[0].TexCoord = Sprite.TextureRect.TopLeft();
            Vertex[1].TexCoord = Sprite.TextureRect.TopRight();
            Vertex[2].TexCoord = Sprite.TextureRect.BottomRight();
            Vertex[3].TexCoord = Sprite.TextureRect.BottomLeft();

            //

            auto Index = IndexBuffer->Map<uint32_t>(DrawIndex * sizeof(uint16_t));

            Index[0] = 0;
            Index[1] = 1;
            Index[2] = 2;

            Index[3] = 0;
            Index[4] = 2;
            Index[5] = 3;

            //

            DrawIndex++;

            IndexBuffer->Unmap();
            VertexBuffer->Unmap();
        };

        //

        if (m_SpriteQuery.is_true())
        {
            m_SpriteQuery.each(DrawToBuffer);
        }
    }
} // namespace Neon::RG