#pragma once

#include <Core/Neon.hpp>
#include <Core/BitMask.hpp>
#include <Core/String.hpp>
#include <RHI/Resource/Common.hpp>
#include <vector>

namespace Neon::RHI
{
    enum class ShaderStage : uint8_t
    {
        Compute,
        Vertex,
        Hull,
        Domain,
        Geometry,
        Pixel,

        Count
    };

    enum class EShaderCompileFlags : uint8_t
    {
        // Enable for debug build
        Debug,

        _Last_Enum
    };
    using MShaderCompileFlags = Bitmask<EShaderCompileFlags>;

    enum class ShaderProfile : uint8_t
    {
        SP_5_0,
        SP_5_1,
        SP_6_0,
        SP_6_1,
        SP_6_2,
        SP_6_3,
        SP_6_4,
        SP_6_5,
        SP_6_6,

        Count
    };

    struct ShaderCompileDesc
    {
        std::vector<std::pair<String, String>> Defines;

        ShaderProfile Profile = ShaderProfile::SP_6_5;
        ShaderStage   Stage;

        StringU8 SourceCode;
        String   EntryPoint;

        MShaderCompileFlags Flags;
    };

    class IShader
    {
    public:
        struct ByteCode
        {
            uint8_t* Data;
            size_t   Size;
        };

        [[nodiscard]] static IShader* Create(
            const ByteCode& CompiledCode);

        [[nodiscard]] static IShader* Create(
            const ShaderCompileDesc& Desc);

        virtual ~IShader() = default;

        /// <summary>
        /// Create input layout to graphics / rawlayout
        /// </summary>
        virtual void CreateInputLayout(
            ShaderInputLayout& Layout) = 0;

        /// <summary>
        /// Create output layout rawlayout
        /// </summary>
        virtual void CreateOuputLayout(
            ShaderInputLayout& Layout) = 0;

        /// <summary>
        /// Get shader bytecode
        /// </summary>
        [[nodiscard]] virtual ByteCode GetByteCode() = 0;
    };
} // namespace Neon::RHI
