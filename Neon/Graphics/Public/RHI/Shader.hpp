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
        Unknown,
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

    static constexpr auto MShaderCompileFlags_Default = MShaderCompileFlags{};

    enum class ShaderProfile : uint8_t
    {
        SP_6_0,
        SP_6_1,
        SP_6_2,
        SP_6_3,
        SP_6_4,
        SP_6_5,
        SP_6_6,

        Count
    };

    struct ShaderMacros
    {
        std::vector<std::pair<String, String>> Defines;

        /// <summary>
        /// Append new macro
        /// </summary>
        void Append(
            const String& Name,
            const String& Value)
        {
            Defines.emplace_back(Name, Value);
        }

        /// <summary>
        /// Get all macros
        /// </summary>
        [[nodiscard]] const auto& Get() const noexcept
        {
            return Defines;
        }

        /// <summary>
        /// Get macro count
        /// </summary>
        [[nodiscard]] size_t Size() const noexcept
        {
            return Defines.size();
        }
    };

    struct ShaderCompileDesc
    {
        ShaderMacros Macros;

        ShaderProfile Profile = ShaderProfile::SP_6_5;
        ShaderStage   Stage   = ShaderStage::Unknown;

        MShaderCompileFlags Flags = MShaderCompileFlags_Default;
    };

    class IShader
    {
    public:
        struct ByteCode
        {
            uint8_t* Data;
            size_t   Size;
        };

        [[nodiscard]] static UPtr<IShader> Create(
            std::unique_ptr<uint8_t[]> Data,
            size_t                     DataSize);

        [[nodiscard]] static UPtr<IShader> Create(
            StringU8View             SourceCode,
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
