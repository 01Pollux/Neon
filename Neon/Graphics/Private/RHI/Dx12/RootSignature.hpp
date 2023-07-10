#pragma once
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <RHI/RootSignature.hpp>
#include <Core/SHA256.hpp>

namespace Neon::RHI
{
    /// <summary>
    /// Cast Neon shader visbility to D3D12_SHADER_VISIBILITY
    /// </summary>
    [[nodiscard]] D3D12_SHADER_VISIBILITY CastShaderVisibility(
        ShaderVisibility Visibility);

    /// <summary>
    /// Cast D3D12_SHADER_VISIBILITY to Neon shader visbility
    /// </summary>
    [[nodiscard]] ShaderVisibility CastShaderVisibility(
        D3D12_SHADER_VISIBILITY Visibility);

    //

    /// <summary>
    /// Cast Neon root parameter type to D3D12_ROOT_PARAMETER_TYPE
    /// </summary>
    [[nodiscard]] D3D12_DESCRIPTOR_RANGE_FLAGS CastRootDescriptorTableFlags(
        const MRootDescriptorTableFlags& Flags);

    /// <summary>
    /// Cast D3D12_ROOT_PARAMETER_TYPE to Neon root parameter type
    /// </summary>
    [[nodiscard]] MRootDescriptorTableFlags CastRootDescriptorTableFlags(
        D3D12_DESCRIPTOR_RANGE_FLAGS Flags);

    //

    /// <summary>
    /// Cast Neon root parameter type to D3D12_ROOT_PARAMETER_TYPE
    /// </summary>
    [[nodiscard]] D3D12_ROOT_DESCRIPTOR_FLAGS CastRootDescriptorFlags(
        const MRootDescriptorFlags& Flags);

    /// <summary>
    /// Cast D3D12_ROOT_PARAMETER_TYPE to Neon root parameter type
    /// </summary>
    [[nodiscard]] MRootDescriptorFlags CastRootDescriptorFlags(
        D3D12_ROOT_DESCRIPTOR_FLAGS Flags);

    //

    class Dx12RootSignature final : public IRootSignature
    {
    public:
        Dx12RootSignature(
            RootSignatureBuilder                         Builder,
            uint32_t                                     ResourceCountInDescriptor,
            uint32_t                                     SamplerCountInDescriptor,
            const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& SignatureDesc,
            SHA256::Bytes&&                              Hash);

        uint32_t GetResourceCountInDescriptor() override;

        uint32_t GetSamplerCountInDescriptor() override;

        /// <summary>
        /// Get the underlying D3D12 root signature
        /// </summary>
        [[nodiscard]] ID3D12RootSignature* Get();

        /// <summary>
        /// Get the root signature hash digest
        /// </summary>
        [[nodiscard]] const SHA256::Bytes& GetHash() const;

    private:
        WinAPI::ComPtr<ID3D12RootSignature> m_RootSignature;
        SHA256::Bytes                      m_Hash;

        uint32_t m_ResourceCountInDescriptor = 0,
                 m_SamplerCountInDescriptor  = 0;
    };

    class Dx12RootSignatureCache
    {
    public:
        /// <summary>
        /// Clear the root signature cache and release all cached root signatures
        /// </summary>
        static void Flush();

        /// <summary>
        /// Get or save root signature from cache
        /// </summary>
        [[nodiscard]] static Ptr<IRootSignature> Load(
            const RootSignatureBuilder& Builder);

    private:
        struct BuildResult
        {
            SHA256::Bytes                                     Digest;
            std::list<std::vector<CD3DX12_DESCRIPTOR_RANGE1>> RangesList;
            std::vector<CD3DX12_ROOT_PARAMETER1>              Parameters;
            std::vector<CD3DX12_STATIC_SAMPLER_DESC>          StaticSamplers;
            D3D12_ROOT_SIGNATURE_FLAGS                        Flags;

            BuildResult() = default;
            NEON_CLASS_NO_COPY(BuildResult);
            NEON_CLASS_MOVE(BuildResult);
            ~BuildResult() = default;
        };

        /// <summary
        /// Get or save root signature from cache
        /// </summary>
        [[nodiscard]] static BuildResult Build(
            const RootSignatureBuilder& Builder,
            uint32_t&                   ResourceCountInDescriptor,
            uint32_t&                   SamplerCountInDescriptor);
    };
} // namespace Neon::RHI