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
            InputLayoutDesc*    GraphicsLayout = nullptr,
            MBuffer::RawLayout* Layout         = nullptr) override;

        /// <summary>
        /// Create output layout rawlayout
        /// </summary>
        void CreateOuputLayout(
            MBuffer::RawLayout& Layout) override;

        /// <summary>
        /// Get shader bytecode
        /// </summary>
        [[nodiscard]] ByteCode GetByteCode() override;

    private:
        std::unique_ptr<uint8_t[]> m_ShaderData;
        size_t                     m_DataSize;
    };
} // namespace Neon::RHI
