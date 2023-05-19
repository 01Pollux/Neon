#pragma once
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <RHI/RootSignature.hpp>

namespace Neon::RHI
{
    class Dx12RootSignature final : public IRootSignature
    {
    public:
        Dx12RootSignature(
            const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& SignatureDesc,
            uint32_t                                     ResourceCount,
            uint32_t                                     SamplerCount);

        ~Dx12RootSignature();

        uint32_t GetResourceCount() override;

        uint32_t GetSamplerCount() override;

    private:
        ID3D12RootSignature* m_RootSignature;
        uint32_t             m_ResourceCount;
        uint32_t             m_SamplerCount;
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
        [[nodiscard]] static Ptr<Dx12RootSignature> Load(
            const RootSignatureBuilder& Builder);
    };
} // namespace Neon::RHI