#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>

namespace Neon::RHI
{
    D3D_ROOT_SIGNATURE_VERSION Dx12DeviceFeatures::GetRootSignatureVersion() const
    {
        return m_RootSignatureVersion;
    }

    uint32_t Dx12DeviceFeatures::MaxDescriptorHeapSize(
        bool Sampler) const
    {
        return m_DescriptorHeapSize[Sampler ? 1 : 0];
    }

    void Dx12DeviceFeatures::Initialize(
        ID3D12Device* Device)
    {
        CD3DX12FeatureSupport FeatureSupport;
        ThrowIfFailed(FeatureSupport.Init(Device));

        m_RootSignatureVersion = FeatureSupport.HighestRootSignatureVersion();

        m_DescriptorHeapSize[0] = FeatureSupport.MaxViewDescriptorHeapSize();
        m_DescriptorHeapSize[1] = FeatureSupport.MaxSamplerDescriptorHeapSize();
    }
} // namespace Neon::RHI