#include <Runtime/EntryPoint.hpp>
#include <Runtime/Types/WorldRuntime.hpp>
#include <Asset/Packs/Directory.hpp>

#include <Asset/Storage.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

using namespace Neon;

const char* ShaderCode = R"(
struct PerFrameData
{
	matrix View;
	matrix Projection;
	matrix ViewProjection;
	matrix ViewInverse;
	matrix ProjectionInverse;
	matrix ViewProjectionInverse;
	
	float3 CameraPosition;
	float3 CameraDirection;
	float3 CameraUp;
};

struct PerObjectData
{
	matrix World;
	float4 Color;
	int TextureIndex;
};

//

struct VSInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEX_COORD;
	int SpriteIndex : SPRITE_INDEX;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEX_COORD;
	nointerpolation int SpriteIndex : SPRITE_INDEX;
};

// --------------------
// Global
// --------------------

ConstantBuffer<PerFrameData> g_FrameData : register(b0, space0);
StructuredBuffer<PerObjectData> g_SpriteData : register(t0, space1);

// --------------------
// Vertex Shader
// --------------------

VSOutput VS_Main(VSInput Input)
{
	VSOutput Output;
	Output.Position = float4(Input.Position, 1.0f);
	Output.Position = mul(Output.Position, g_SpriteData[Input.SpriteIndex].World);
	Output.Position = mul(Output.Position, g_FrameData.ViewProjection);
	Output.TexCoord = Input.TexCoord;
	Output.SpriteIndex = Input.SpriteIndex;
	return Output;
}

// --------------------
// Pixel Shader
// --------------------

Texture2D p_SpriteTextures[] : register(t0, space0);

SamplerState p_Sampler_PointWrap : register(s0, space0);
SamplerState p_Sampler_PointClamp : register(s1, space0);
SamplerState p_Sampler_LinearWrap : register(s2, space0);
SamplerState p_Sampler_LinearClamp : register(s3, space0);
SamplerState p_Sampler_AnisotropicWrap : register(s4, space0);
SamplerState p_Sampler_AnisotropicClamp : register(s5, space0);

float4 PS_Main(VSOutput Input) : SV_TARGET
{
	int TextureIndex = g_SpriteData[Input.SpriteIndex].TextureIndex;
	float4 Color = p_SpriteTextures[TextureIndex].Sample(p_Sampler_PointWrap, Input.TexCoord);
	return Color * g_SpriteData[Input.SpriteIndex].Color;
}
)";

class RuntimeSample : public Runtime::DefaultGameEngine
{
public:
    void Initialize(
        Config::EngineConfig Config) override
    {
        DefaultGameEngine::Initialize(std::move(Config));

        Asset::Storage::Mount(std::make_unique<Asset::DirectoryAssetPackage>("Contents"));

        auto TestShaderId = Asset::Handle::FromString("7427990f-9be1-4a23-aad5-1b99f00c29fd");
        auto Shader       = std::make_shared<Asset::ShaderAsset>(ShaderCode, TestShaderId, "Shaders/Sprite2D.hlsl");

        Asset::Storage::SaveAsset({ .Asset = Shader });
        Asset::Storage::ExportAll();
        /*
         RegisterInterface<Runtime::IEngineRuntime, Runtime::EngineWorldRuntime>();*/
    }
};

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false },
        //.Renderer{ .Device = { .EnableGpuBasedValidation = true } }
    };
    return RunEngine<RuntimeSample>(std::move(Config));
}
