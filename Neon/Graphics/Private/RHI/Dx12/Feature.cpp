#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>

namespace Neon::RHI
{
    D3D_ROOT_SIGNATURE_VERSION Dx12DeviceFeatures::GetRootSignatureVersion() const
    {
        return m_RootSignatureVersion;
    }

    void Dx12DeviceFeatures::Initialize(
        ID3D12Device* Device)
    {
        CD3DX12FeatureSupport FeatureSupport;
        ThrowIfFailed(FeatureSupport.Init(Device));

        m_RootSignatureVersion = FeatureSupport.HighestRootSignatureVersion();
    }
} // namespace Neon::RHI