#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    Ptr<IRootSignature> IRootSignature::Create(
        const RootSignatureBuilder& Builder)
    {
        return Dx12RootSignatureCache::Load(Builder);
    }

    Dx12RootSignature::Dx12RootSignature(
        const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& SignatureDesc,
        uint32_t                                     ResourceCount,
        uint32_t                                     SamplerCount) :
        m_ResourceCount(ResourceCount),
        m_SamplerCount(SamplerCount)
    {
        Win32::ComPtr<ID3DBlob> SignatureBlob;
        Win32::ComPtr<ID3DBlob> ErrorBlob;

        auto  Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        auto& Features   = Dx12RenderDevice::Get()->GetFeatures();

        HRESULT Result = D3DX12SerializeVersionedRootSignature(
            &SignatureDesc,
            Features.GetRootSignatureVersion(),
            &SignatureBlob,
            &ErrorBlob);

        if (FAILED(Result))
        {
            auto Error = static_cast<char*>(ErrorBlob->GetBufferPointer());
            NEON_VALIDATE(false, Error);
        }

        ThrowIfFailed(Dx12Device->CreateRootSignature(
            0,
            SignatureBlob->GetBufferPointer(),
            SignatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_RootSignature)));
    }

    Dx12RootSignature::~Dx12RootSignature()
    {
        if (m_RootSignature)
        {
            m_RootSignature->Release();
            m_RootSignature = nullptr;
        }
    }

    uint32_t Dx12RootSignature::GetResourceCount()
    {
        return m_ResourceCount;
    }

    uint32_t Dx12RootSignature::GetSamplerCount()
    {
        return m_SamplerCount;
    }

    //

    Ptr<Dx12RootSignature> Dx12RootSignatureCache::Load(
        const RootSignatureBuilder& Builder)
    {
        return Ptr<Dx12RootSignature>();
    }
} // namespace Neon::RHI