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

/*

// ShaderLibrary File Format
// 1. Header:
// ShaderId
// ModuleName
// OffsetToCompressedShader
// CompressedShaderSize
//
// ...
//
// 0xFFFFFFFF
//
// 2. Body:
// CompressedShader
// Just text code for shaders

// ShaderModule File Format
// Hash
// CompiledBinary
// ...


// ShaderId must exist in ShaderLibrary
// ShaderModule is a file with name generated from ModuleName in header
auto ShaderModule = ShaderLibrary->LoadModule(ShaderId(0x00000001));

ShaderLibrary->SetModule(ShaderId(0x00000001), ModuleName, Code);

// Shader stage can exist in ShaderModule or not (if not, it will be compiled and added to ShaderModule)
auto Future = ShaderModule->LoadStage(ShaderStage::Vertex, ShaderProfile::SP_6_0, ShaderMacros{});

// Remove all loaded binaries from this shader module
ShaderModule->Optimize();

// Remove all loaded binaries from all shader modules
ShaderLibrary->Optimize();





PreloadShader:


LoadShader:
    auto Hash = ShaderLibrary->GetHash(Asset, Stage, Flags, Profile, Macros);
    if (ShaderLibrary->Contains(Hash))
    {
        return ShaderLibrary->GetShader(Hash);
    }



    auto Shader = ShaderLibrary->CreateShader(Asset, Stage, Flags, Profile, Macros);
    ShaderLibrary->AddShader(Asset, Stage, Flags, Profile, Macros, Shader);
    return Shader;
*/

#include <chrono>

using namespace Neon;

class TestGameEngine : public Runtime::DefaultGameEngine
{
public:
    Asset::ShaderLibraryAsset ShaderLibrary;

    void Initialize(
        const Config::EngineConfig& Config) override
    {
        Runtime::DefaultGameEngine::Initialize(Config);

        auto t1 = std::chrono::high_resolution_clock::now();

        ShaderLibrary.SetModule(Asset::ShaderModuleId(1), "Sprite", Shader);
        ShaderLibrary.SetModule(Asset::ShaderModuleId(2), "Other", Shader);

        auto Mod1 = ShaderLibrary.LoadModule(Asset::ShaderModuleId(1));

        auto p = Mod1->LoadStage(
            RHI::ShaderStage::Vertex,
            RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile::SP_6_0);

        auto a = p.get();

        auto t2 = std::chrono::high_resolution_clock::now();

        auto xp = Mod1->LoadStage(
            RHI::ShaderStage::Vertex,
            RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile::SP_6_0);

        a = xp.get();

        auto t3 = std::chrono::high_resolution_clock::now();

        auto x = Mod1->LoadStage(
            RHI::ShaderStage::Pixel,
            RHI::MShaderCompileFlags_Default,
            RHI::ShaderProfile::SP_6_0);

        a = x.get();

        auto t4 = std::chrono::high_resolution_clock::now();

        printf("t2-t1: %lf\n", std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count());
        printf("t3-t2: %lf\n", std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2).count());
        printf("t4-t3: %lf\n", std::chrono::duration_cast<std::chrono::duration<double>>(t4 - t3).count());
    }
};

#include <cppcoro/generator.hpp>
#include <iostream>

cppcoro::generator<const std::uint64_t> fibonacci()
{
    std::uint64_t a = 0, b = 1;
    while (true)
    {
        co_yield b;
        auto tmp = a;
        a        = b;
        b += tmp;
    }
}

void usage()
{
    for (auto i : fibonacci())
    {
        if (i > 1'000'000)
            break;
        std::cout << i << std::endl;
    }
}

NEON_MAIN(Argc, Argv)
{
    usage();
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false }
    };
    return RunEngine<TestGameEngine>(Config);
}
