#include <EnginePCH.hpp>
#include <Asset/Handlers/Model.hpp>
#include <RHI/Material/Shared.hpp>
#include <Utils/Struct.hpp>

#ifndef NEON_DIST
#include <Runtime/GameEngine.hpp>

#include <AssImp/Importer.hpp>
#include <AssImp/postprocess.h>
#include <AssImp/scene.h>

#include <AssImp/Logger.hpp>
#include <AssImp/LogStream.hpp>
#include <AssImp/DefaultLogger.hpp>
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
        aiNode*                   AINode,
        Mdl::Model::SubmeshList&  Submeshes,
        Mdl::Model::MeshNodeList& MeshNode,
        uint32_t                  ParentIndex     = std::numeric_limits<uint32_t>::max(),
        const Matrix4x4&          ParentTransform = Mat::Identity<Matrix4x4>)
    {
        Mdl::MeshNode Node{
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
            uint32_t ChildIndex = static_cast<uint32_t>(MeshNode.size());
            MeshNode[NodeIndex].Children.push_back(ChildIndex);
            TraverseAISubMesh(AINode->mChildren[i], Submeshes, MeshNode, NodeIndex, FinalTransform);
        }
    }

    struct AssimpLogStream : public Assimp::LogStream
    {
        static void InitializeOnce();

        virtual void write(
            const char* Message) override
        {
            StringU8 MessageStr(Message);
            StringU8 Type;

            size_t Offset = 0;

            // Text string is built like this: "Type, Message", split it up.
            size_t TypeEnd = MessageStr.find_first_of(", ");
            if (TypeEnd != StringU8::npos) [[likely]]
            {
                Type   = MessageStr.substr(0, TypeEnd);
                Offset = TypeEnd + 2;

                // Remove the space after the comma.
                if (Offset < MessageStr.size() && MessageStr[Offset] == ' ') [[likely]]
                {
                    Offset++;
                }
                MessageStr = MessageStr.substr(Offset);
            }

            static std::map<StringU8, Logger::LogSeverity> Severities{
                { "Warn", Logger::LogSeverity::Warning },
                { "Info", Logger::LogSeverity::Info },
                { "Debug", Logger::LogSeverity::Trace }
            };

            Logger::LogSeverity Severity = Logger::LogSeverity::Error;

            auto Iter = Severities.find(Type);
            if (Iter != Severities.end()) [[likely]]
            {
                Severity = Iter->second;
            }

            Neon::Logger::LogTag(Severity, "Assimp", MessageStr);
        }
    };

    void AssimpLogStream::InitializeOnce()
    {
        static AssimpLogStream s_AssimpLogStream;
        if (Assimp::DefaultLogger::isNullLogger()) [[unlikely]]
        {
#if NEON_DEBUG
            Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
            Assimp::DefaultLogger::get()->attachStream(&s_AssimpLogStream, Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Warn | Assimp::Logger::Err);
#else
            Assimp::DefaultLogger::create("", Assimp::Logger::NORMAL);
            Assimp::DefaultLogger::get()->attachStream(&s_AssimpLogStream, Assimp::Logger::Warn | Assimp::Logger::Err);
#endif
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

        Ptr<Mdl::Model> Model;

        auto Extension = Path.substr(ExtensionIndex + 1);
        switch (StringUtils::Hash(Extension))
        {
        case StringUtils::Hash("nmdl"):
            break;

#ifndef NEON_DIST
        default:
        {
            AssimpLogStream::InitializeOnce();

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

            Mdl::Model::SubmeshList    Submeshes;
            Mdl::Model::MeshNodeList   MeshNodes;
            Mdl::Model::MaterialsTable Materials;

            std::vector<Mdl::MeshVertex> Vertices;
            std::vector<uint32_t>        Indices;

            //

            using TextureMap         = std::unordered_map<aiTextureType, RHI::SSyncGpuResource>;
            using MaterialTextureMap = std::unordered_map<RHI::IMaterial*, TextureMap>;

            // Deferred loading of textures until we process all meshes.
            MaterialTextureMap MaterialsToSet;

            std::map<aiTextureType, const char*> TextureKvMap{
                std::pair{ aiTextureType_DIFFUSE, "p_AlbedoMap" },
                std::pair{ aiTextureType_NORMALS, "p_NormalMap" },
                std::pair{ aiTextureType_EMISSIVE, "p_EmissiveMap" }
            };

            std::array MaterialKvList{
                std::tuple{ Structured::Type::Float3, "$clr.diffuse", "Color_Albedo" },
                std::tuple{ Structured::Type::Float3, "$clr.specular", "Color_Specular" },
                std::tuple{ Structured::Type::Float4, "$clr.emissive", "Color_Emissive" }
            };

            auto& ThreadPool = Runtime::GameEngine::Get()->GetThreadPool();

            //

            // Process materials.
            std::future<void> LoadMaterialTask;
            if (AIScene->HasMaterials())
            {
                LoadMaterialTask = ThreadPool.enqueue(
                    [&]
                    {
                        auto& WhiteTexture = RHI::IGpuResource::GetDefaultTexture(RHI::DefaultTextures::White_2D);

                        Materials.reserve(AIScene->mNumMaterials);
                        auto LitMaterial = RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::Lit);

                        for (uint32_t i = 0; i < AIScene->mNumMaterials; i++)
                        {
                            aiMaterial* AIMaterial = AIScene->mMaterials[i];
                            auto&       Material   = Materials.emplace_back(LitMaterial->CreateInstance());

                            NEON_TRACE_TAG("Model", "Loading material {}", AIMaterial->GetName().C_Str());

                            TextureMap TexturesToSet;
                            aiString   TexturePath;

                            for (auto& [Type, Tag] : TextureKvMap)
                            {
                                if (AIMaterial->GetTexture(Type, 0, &TexturePath) == AI_SUCCESS)
                                {
                                    NEON_TRACE_TAG("Model", "Loading texture '{}'", TexturePath.C_Str());

#ifndef NEON_DIST
                                    auto MatTextureName = TexturePath.length ? StringUtils::Transform<String>(StringU8View(TexturePath.data, TexturePath.length)) : STR("Unnamed Texture");
                                    auto TextureName    = StringUtils::Format(STR("Assimp_Texture::{}"), std::move(MatTextureName));
                                    auto TextureNamePtr = TextureName.c_str();
#else
                                    const wchar_t* TextureNamePtr = nullptr;
#endif

                                    if (auto AITexture = AIScene->GetEmbeddedTexture(TexturePath.C_Str()))
                                    {
                                        std::array Subresources{
                                            RHI::ComputeSubresource(
                                                RHI::EResourceFormat::R8G8B8A8_UNorm,
                                                AITexture->pcData,
                                                AITexture->mWidth,
                                                AITexture->mHeight)
                                        };
                                        auto Texture = RHI::SSyncGpuResource(
                                            RHI::ResourceDesc::Tex2D(
                                                RHI::EResourceFormat::R8G8B8A8_UNorm,
                                                AITexture->mWidth,
                                                AITexture->mHeight,
                                                1),
                                            Subresources,
                                            TextureNamePtr);
                                        TexturesToSet.emplace(Type, std::move(Texture));
                                    }
                                    else
                                    {
                                        std::fstream File(TexturePath.C_Str(), std::ios::in | std::ios::binary);
                                        if (File.is_open())
                                        {
                                            std::vector<uint8_t> Data((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());

                                            RHI::TextureRawImage ImageInfo{
                                                .Data = Data.data(),
                                                .Size = Data.size(),
                                                .Type = RHI::TextureRawImage::Format::Png
                                            };

                                            TexturesToSet.emplace(Type, RHI::SSyncGpuResource(ImageInfo, TextureNamePtr, RHI::MResourceState_AllShaderResource));
                                        }
                                        else
                                        {
                                            NEON_WARNING("Failed to load texture '{}'", TexturePath.C_Str());
                                        }
                                    }
                                }
                            }

                            for (auto& [Type, AIType, Tag] : MaterialKvList)
                            {
                                switch (Type)
                                {
                                case Structured::Type::Float3:
                                {
                                    aiColor3D Color;
                                    if (AIMaterial->Get(AIType, 0, 0, Color) == AI_SUCCESS)
                                    {
                                        Material->Set(Tag, Color);
                                    }
                                    break;
                                }
                                case Structured::Type::Float4:
                                {
                                    aiColor4D Color;
                                    if (AIMaterial->Get(AIType, 0, 0, Color) == AI_SUCCESS)
                                    {
                                        Material->Set(Tag, Color);
                                    }
                                    break;
                                }
                                default:
                                {
                                    NEON_WARNING_TAG("Model", "Unsupported data type: {}", AIType);
                                }
                                }
                            }

                            MaterialsToSet.emplace(Material.get(), std::move(TexturesToSet));
                        }
                    });
            }

            //

            RHI::USyncGpuResource VertexBuffer, IndexBuffer;

            // Process nodes
            if (AIScene->HasMeshes())
            {
                uint32_t VerticesCount = 0, IndicesCount = 0;
                Submeshes.reserve(AIScene->mNumMeshes);

                for (uint32_t i = 0; i < AIScene->mNumMeshes; i++)
                {
                    aiMesh* AIMesh = AIScene->mMeshes[i];

                    uint32_t VertexCount = AIMesh->mNumVertices;
                    Vertices.reserve(Vertices.size() + VertexCount);

                    Vector3 Min(std::numeric_limits<float>::max()),
                        Max(std::numeric_limits<float>::lowest());

                    for (uint32_t j = 0; j < VertexCount; j++)
                    {
                        Vector3 Position(AIMesh->mVertices[j].x, AIMesh->mVertices[j].y, AIMesh->mVertices[j].z);
                        Min = glm::min(Min, Position);
                        Max = glm::max(Max, Position);

                        auto& Vertex = Vertices.emplace_back(
                            Mdl::MeshVertex{
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

                    RHI::PrimitiveTopology Toplogy = RHI::PrimitiveTopology::Undefined;
                    switch (AIMesh->mPrimitiveTypes & (aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_TRIANGLE))
                    {
                    case aiPrimitiveType_POINT:
                        Toplogy = RHI::PrimitiveTopology::PointList;
                        break;
                    case aiPrimitiveType_LINE:
                        Toplogy = RHI::PrimitiveTopology::LineList;
                        break;
                    case aiPrimitiveType_TRIANGLE:
                        Toplogy = RHI::PrimitiveTopology::TriangleList;
                        break;
                    }

                    Geometry::AABB Box{
                        .Center  = (Max + Min) * 0.5f,
                        .Extents = (Max - Min) * 0.5f
                    };

                    Submeshes.emplace_back(
                        Mdl::SubMeshData{
                            .AABB          = std::move(Box),
                            .VertexCount   = VertexCount,
                            .IndexCount    = IndexCount,
                            .VertexOffset  = VerticesCount,
                            .IndexOffset   = IndicesCount,
                            .MaterialIndex = AIMesh->mMaterialIndex,
                            .Topology      = Toplogy });

                    VerticesCount += VertexCount;
                    IndicesCount += IndexCount;

                    NEON_VALIDATE(AIMesh->HasPositions(), "Mesh has no positions");
                    NEON_VALIDATE(AIMesh->HasNormals(), "Mesh has no normals");
                }

                MeshNodes.reserve(AIScene->mNumMeshes);
                TraverseAISubMesh(AIScene->mRootNode, Submeshes, MeshNodes);

#ifndef NEON_DIST
                auto SceneName     = AIScene->mName.length ? StringUtils::Transform<String>(StringU8View(AIScene->mName.data, AIScene->mName.length)) : STR("Unnamed Scene");
                auto VtxBufferName = StringUtils::Format(STR("Assimp_VertexBuffer::{}"), SceneName);
                auto IdxBufferName = StringUtils::Format(STR("Assimp_IndexBuffer::{}"), SceneName);

                auto VtxBufferNamePtr = VtxBufferName.c_str();
                auto IdxBufferNamePtr = IdxBufferName.c_str();
#else
                const wchar_t* VtxBufferNamePtr = nullptr;
                const wchar_t* IdxBufferNamePtr = nullptr;
#endif

                VertexBuffer = RHI::USyncGpuResource::Buffer(
                    sizeof(Mdl::MeshVertex),
                    Vertices.size(),
                    Vertices.data(),
                    VtxBufferNamePtr,
                    {},
                    RHI::MResourceState_AllShaderResource);

                IndexBuffer = RHI::USyncGpuResource::Buffer(
                    sizeof(uint32_t),
                    Indices.size(),
                    Indices.data(),
                    IdxBufferNamePtr,
                    {},
                    RHI::MResourceState_AllShaderResource);
            }

            //

            if (LoadMaterialTask.valid())
            {
                LoadMaterialTask.get();
            }

            for (auto& [Material, Textures] : MaterialsToSet)
            {
                for (auto& [Type, Texture] : Textures)
                {
                    Material->SetTexture(TextureKvMap[Type], Texture.Get());
                }
            }

            Model = std::make_shared<Mdl::Model>(
                std::move(VertexBuffer),
                std::move(IndexBuffer),
                false,
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