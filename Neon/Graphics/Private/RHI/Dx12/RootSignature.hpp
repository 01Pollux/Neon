#pragma once
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <RHI/RootSignature.hpp>
#include <Crypto/SHA256.hpp>

namespace Neon::RHI
{
    class IShader;

    //

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
            const wchar_t*              Name,
            const RootSignatureBuilder& Builder,
            const void*                 BlobData,
            size_t                      BlobSize,
            Crypto::Sha256::Bytes&&     Hash);

        /// <summary>
        /// Get the underlying D3D12 root signature
        /// </summary>
        [[nodiscard]] ID3D12RootSignature* Get();

        /// <summary>
        /// Get the root signature hash digest
        /// </summary>
        [[nodiscard]] const Crypto::Sha256::Bytes& GetHash() const;

    private:
        WinAPI::ComPtr<ID3D12RootSignature> m_RootSignature;
        Crypto::Sha256::Bytes               m_Hash;
    };

    class Dx12RootSignatureCache
    {
    public:
        /// <summary>
        /// Load common root signatures
        /// </summary>
        static void Load();

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
            const RootSignatureBuilder& Builder);
    };
} // namespace Neon::RHI