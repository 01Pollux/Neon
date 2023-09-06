#include <EnginePCH.hpp>
#include <Asset/Handlers/Model.hpp>
#include <Renderer/Material/Shared.hpp>

#ifndef NEON_DIST
#include <AssImp/Importer.hpp>
#include <AssImp/postprocess.h>
#include <AssImp/scene.h>

#include <vector>
#endif

#include <Log/Logger.hpp>

namespace Neon::Asset
{
#ifndef NEON_DIST
    static const uint32_t s_MeshImportFlags =
        aiProcess_CalcTangentSpace |    // Create binormals/tangents just in case
        aiProcess_ConvertToLeftHanded | // Make sure we're left handed
        aiProcess_Triangulate |         // Make sure we're triangles
        aiProcess_SortByPType |         // Split meshes by primitive type
        aiProcess_GenNormals |          // Make sure we have legit normals
        aiProcess_GenUVCoords |         // Convert UVs if required
        // aiProcess_OptimizeGraph |
        aiProcess_OptimizeMeshes | // Batch draws where possible
        aiProcess_JoinIdenticalVertices |
        aiProcess_LimitBoneWeights |      // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
        aiProcess_ValidateDataStructure | // Validation
        aiProcess_GlobalScale;            // e.g. convert cm to m for fbx import (and other formats where cm is native)

    //

    /// <summary>
    /// Convert an AssImp matrix to a Neon matrix.
    /// </summary>
    [[nodiscard]] static Matrix4x4 FromAiMatrix(
        const aiMatrix4x4& Matrix)
    {
        return Matrix4x4(
            Matrix.a1, Matrix.b1, Matrix.c1, Matrix.d1,
            Matrix.a2, Matrix.b2, Matrix.c2, Matrix.d2,
            Matrix.a3, Matrix.b3, Matrix.c3, Matrix.d3,
            Matrix.a4, Matrix.b4, Matrix.c4, Matrix.d4);
    }

    /// <summary>
    /// Traverse an AssImp node and add it to the model as a mesh node.
    /// </summary>
    static void TraverseAISubMesh(
        aiNode*                        AINode,
        Renderer::Model::SubmeshList&  Submeshes,
        Renderer::Model::MeshNodeList& MeshNode,
        uint32_t                       ParentIndex     = 0,
        const Matrix4x4&               ParentTransform = Mat::Identity<Matrix4x4>)
    {
        Renderer::MeshNode Node{
            .Parent    = ParentIndex,
            .Transform = FromAiMatrix(AINode->mTransformation),
            .Name      = StringU8(AINode->mName.data, AINode->mName.length)
        };

        Matrix4x4 FinalTransform = ParentTransform * Node.Transform;
        Node.Submeshes.reserve(AINode->mNumMeshes);

        for (uint32_t i = 0; i < AINode->mNumMeshes; ++i)
        {
            uint32_t MeshIndex             = AINode->mMeshes[i];
            Submeshes[MeshIndex].Transform = FinalTransform;
            Node.Submeshes.push_back(MeshIndex);
        }

        uint32_t NodeIndex = static_cast<uint32_t>(MeshNode.size());
        MeshNode.emplace_back(std::move(Node));
        MeshNode[NodeIndex].Children.reserve(AINode->mNumChildren);

        for (uint32_t i = 0; i < AINode->mNumChildren; i++)
        {
            uint32_t ChildIndex = static_cast<uint32_t>(MeshNode.size()) - 1;
            MeshNode[NodeIndex].Children.push_back(ChildIndex);
            TraverseAISubMesh(AINode->mChildren[i], Submeshes, MeshNode, NodeIndex, FinalTransform);
        }
    }

#endif

    bool ModelAsset::Handler::CanHandle(
        const Ptr<IAsset>& Asset)
    {
        return dynamic_cast<ModelAsset*>(Asset.get());
    }

    Ptr<IAsset> ModelAsset::Handler::Load(
        std::istream& Stream,
        const Asset::DependencyReader&,
        const Handle&        AssetGuid,
        StringU8             Path,
        const AssetMetaData& LoaderData)
    {
        // Get extension from path.
        size_t ExtensionIndex = Path.find_last_of('.');
        if (ExtensionIndex == StringU8::npos)
        {
            return nullptr;
        }

        Ptr<Renderer::Model> Model;

        auto Extension = Path.substr(ExtensionIndex + 1);
        switch (StringUtils::Hash(Extension))
        {
        case StringUtils::Hash("nmdl"):
            break;

#ifndef NEON_DIST
        default:
        {
            Assimp::Importer Importer;
            Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, true);

            const aiScene* AIScene = nullptr;
            {
                std::vector<uint8_t> Buffer{ std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>() };
                AIScene = Importer.ReadFileFromMemory(Buffer.data(), Buffer.size(), s_MeshImportFlags, Extension.c_str());
            }

            if (!AIScene)
            {
                return nullptr;
            }

            Renderer::Model::SubmeshList    Submeshes;
            Renderer::Model::MeshNodeList   MeshNodes;
            Renderer::Model::MaterialsTable Materials;

            std::vector<Renderer::MeshVertex> Vertices;
            std::vector<uint32_t>             Indices;

            if (AIScene->HasMeshes())
            {
                uint32_t VerticesCount = 0, IndicesCount = 0;
                Submeshes.reserve(AIScene->mNumMeshes);

                for (uint32_t i = 0; i < AIScene->mNumMeshes; i++)
                {
                    aiMesh* AIMesh = AIScene->mMeshes[i];

                    uint32_t VertexCount = AIMesh->mNumVertices;
                    Vertices.reserve(Vertices.size() + VertexCount);

                    AABoundingBox3D Box;
                    for (uint32_t j = 0; j < VertexCount; j++)
                    {
                        Vector3 Position(AIMesh->mVertices[j].x, AIMesh->mVertices[j].y, AIMesh->mVertices[j].z);
                        Box.Expand(Position);

                        auto& Vertex = Vertices.emplace_back(
                            Renderer::MeshVertex{
                                .Position = Position,
                                .Normal   = Vector3(AIMesh->mNormals[j].x, AIMesh->mNormals[j].y, AIMesh->mNormals[j].z) });

                        if (AIMesh->HasTangentsAndBitangents())
                        {
                            Vertex.Tangent   = Vector3(AIMesh->mTangents[j].x, AIMesh->mTangents[j].y, AIMesh->mTangents[j].z);
                            Vertex.Bitangent = Vector3(AIMesh->mBitangents[j].x, AIMesh->mBitangents[j].y, AIMesh->mBitangents[j].z);
                        }

                        if (AIMesh->HasTextureCoords(0))
                        {
                            Vertex.TexCoord = Vector2(AIMesh->mTextureCoords[0][j].x, AIMesh->mTextureCoords[0][j].y);
                        }
                    }

                    uint32_t OldIndexCount = uint32_t(Indices.size());

                    for (uint32_t j = 0; j < AIMesh->mNumFaces; j++)
                    {
                        aiFace& Face = AIMesh->mFaces[j];
                        for (uint32_t k = 0; k < Face.mNumIndices; k++)
                        {
                            Indices.emplace_back(Face.mIndices[k]);
                        }
                    }

                    uint32_t IndexCount = uint32_t(Indices.size()) - OldIndexCount;

                    Submeshes.emplace_back(
                        Renderer::SubMeshData{
                            .AABB          = std::move(Box),
                            .VertexCount   = VertexCount,
                            .IndexCount    = IndexCount,
                            .VertexOffset  = VerticesCount,
                            .IndexOffset   = IndicesCount,
                            .MaterialIndex = AIMesh->mMaterialIndex });

                    VerticesCount += VertexCount;
                    IndicesCount += IndexCount;

                    NEON_VALIDATE(AIMesh->HasPositions(), "Mesh has no positions");
                    NEON_VALIDATE(AIMesh->HasNormals(), "Mesh has no normals");
                }

                MeshNodes.reserve(AIScene->mNumMeshes);
                MeshNodes.emplace_back();
                TraverseAISubMesh(AIScene->mRootNode, Submeshes, MeshNodes);
            }

            if (AIScene->HasMaterials())
            {
                Materials.reserve(AIScene->mNumMaterials);
                auto LitMaterial = Renderer::SharedMaterials::Get(Renderer::SharedMaterials::Type::Lit);

                for (uint32_t i = 0; i < AIScene->mNumMaterials; i++)
                {
                    aiMaterial* AIMaterial = AIScene->mMaterials[i];
                    auto&       Material   = Materials.emplace_back(LitMaterial->CreateInstance());

                    NEON_TRACE_TAG("Model", "Loading material %s", AIMaterial->GetName().C_Str());

                    aiString TexturePath;
                    for (auto [Type, Tag] : {
                             std::pair{ aiTextureType_DIFFUSE, "Diffuse" },
                             std::pair{ aiTextureType_SPECULAR, "Specular" },
                             std::pair{ aiTextureType_NORMALS, "Normal" },
                             std::pair{ aiTextureType_EMISSIVE, "Emissive" } })
                    {
                        if (AIMaterial->GetTexture(Type, 0, &TexturePath))
                        {
                            if (auto AITexture = AIScene->GetEmbeddedTexture(TexturePath.C_Str()))
                            {
                                auto Texture = RHI::ITexture::Create(
                                    RHI::ResourceDesc::Tex2D(
                                        RHI::EResourceFormat::R8G8B8A8_UNorm,
                                        AITexture->mWidth,
                                        AITexture->mHeight,
                                        1));

                                //
                            }
                            else
                            {
                            }
                        }
                    }
                }
            }
            else
            {
            }

            // TODO: Use IBuffer::Create to create static buffer rather than dynamic buffer
            auto VertexBuffer = UPtr<RHI::IUploadBuffer>(RHI::IUploadBuffer::Create(
                RHI::BufferDesc{
                    .Size    = sizeof(Renderer::MeshVertex) * Vertices.size() }));
            auto IndexBuffer  = UPtr<RHI::IUploadBuffer>(RHI::IUploadBuffer::Create(
                RHI::BufferDesc{
                     .Size    = sizeof(Renderer::MeshVertex) * Vertices.size() }));

            VertexBuffer->Write(0, Vertices.data(), sizeof(Renderer::MeshVertex) * Vertices.size());
            IndexBuffer->Write(0, Indices.data(), sizeof(uint32_t) * Indices.size());

            Model = std::make_shared<Renderer::Model>(
                std::move(VertexBuffer),
                std::move(IndexBuffer),
                std::move(Submeshes),
                std::move(MeshNodes),
                std::move(Materials));
            break;
        }
#endif
        }

        auto Asset = std::make_shared<ModelAsset>(Model, AssetGuid, std::move(Path));
        return Asset;
    }

    void ModelAsset::Handler::Save(
        std::iostream& Stream,
        DependencyWriter&,
        const Ptr<IAsset>& Asset,
        AssetMetaData&     LoaderData)
    {
        auto AssetPtr = static_cast<Asset::ModelAsset*>(Asset.get());
    }
} // namespace Neon::Asset