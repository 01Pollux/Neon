#include <Runtime/EntryPoint.hpp>

#include <Resource/Types/Shader.hpp>

const char* Shader = R"(
struct VSInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	int TexIndex : TEXINDEX;
	float4 Color : COLOR;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD;
	nointerpolation int TexIndex : TEXINDEX;
};

VSOutput VS_Main(VSInput Input)
{
	VSOutput Output;
	Output.Position = float4(Input.Position, 1.0f);
	Output.TexCoord = Input.TexCoord;
	Output.Color = Input.Color;
	Output.TexIndex = Input.TexIndex;
	return Output;
}

Texture2D<float4> Texture : register(t0, space0);
SamplerState Sampler;

float4 PS_Main(VSOutput Input) : SV_TARGET
{
	float4 Color = (float4) 1.f;
	if (Input.TexIndex != -1)
	{
		Color = Texture.Sample(Sampler, Input.TexCoord);
	}
	return Color * Input.Color;
}
)";

#include <chrono>

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/sync_wait.hpp>

using namespace Neon;

class TestGameEngine : public Runtime::DefaultGameEngine
{
public:
    Asset::ShaderLibraryAsset ShaderLibrary;

    void Initialize(
        const Config::EngineConfig& Config) override
    {
        Runtime::DefaultGameEngine::Initialize(Config);

        for (int i = 0; i < 100; ++i)
        {
            std::string str = "Shader" + std::to_string(i);
            ShaderLibrary.SetModule(Asset::ShaderModuleId(1 + i), str, Shader);

            auto Mod1 = ShaderLibrary.LoadModule(Asset::ShaderModuleId(1 + i));

            auto t1 = std::chrono::high_resolution_clock::now();

            auto Shaders =
                cppcoro::sync_wait(
                    cppcoro::when_all(
                        Mod1->LoadStage(
                            RHI::ShaderStage::Vertex,
                            RHI::MShaderCompileFlags_Default,
                            RHI::ShaderProfile::SP_6_0),
                        Mod1->LoadStage(
                            RHI::ShaderStage::Vertex,
                            RHI::MShaderCompileFlags_Default,
                            RHI::ShaderProfile::SP_6_0),
                        Mod1->LoadStage(
                            RHI::ShaderStage::Pixel,
                            RHI::MShaderCompileFlags_Default,
                            RHI::ShaderProfile::SP_6_0)));

            auto t2 = std::chrono::high_resolution_clock::now();

            printf("t2-t1: %lf\n", std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count());
        }
    }
};

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false }
    };
    return RunEngine<TestGameEngine>(Config);
}
