#pragma once

#include <RHI/Shader.hpp>

namespace Neon::RHI
{
    class Dx12Shader final : public IShader
    {
    public:
        Dx12Shader(
            UPtr<uint8_t[]> ShaderData,
            size_t          DataSize);

        /// <summary>
        /// Create input layout to graphics / rawlayout
        /// </summary>
        void CreateInputLayout(
            ShaderInputLayout& Layout) override;

        /// <summary>
        /// Create output layout rawlayout
        /// </summary>
        void CreateOuputLayout(
            ShaderInputLayout& Layout) override;

        /// <summary>
        /// Get shader bytecode
        /// </summary>
        [[nodiscard]] ByteCode GetByteCode() override;

    public:
        /// <summary>
        /// Get group size of compute shader
        /// </summary>
        [[nodiscard]] Vector3U GetComputeGroupSize() const;

        /// <summary>
        /// Get shader's reflection
        /// </summary>
        [[nodiscard]] WinAPI::ComPtr<ID3D12ShaderReflection> GetReflection() const;

    private:
        std::unique_ptr<uint8_t[]> m_ShaderData;
        size_t                     m_DataSize;
    };
} // namespace Neon::RHI
