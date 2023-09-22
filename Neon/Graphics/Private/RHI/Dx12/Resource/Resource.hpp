#pragma once

#include <RHI/Resource/Resource.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Math/Vector.hpp>
#include <future>

namespace Neon::RHI
{
    class Dx12GpuResource : public IGpuResource
    {
    public:
        Dx12GpuResource(
            const ResourceDesc&  Desc,
            const InitDesc&      Init,
            D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE);

        Dx12GpuResource(
            std::future<void>&                  CopyTask,
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
            std::span<const SubresourceDesc>    Subresources,
            const wchar_t*                      Name,
            const RHI::MResourceState&          InitialState);

        Dx12GpuResource(
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            D3D12_RESOURCE_STATES               InitialState,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation);

        ~Dx12GpuResource();

        void QueryFootprint(
            uint32_t              FirstSubresource,
            uint32_t              SubresourceCount,
            size_t                Offset,
            SubresourceFootprint* OutFootprint,
            uint32_t*             NumRows,
            size_t*               RowSizeInBytes,
            size_t*               LinearSize) const override;

        std::future<void> CopyFrom(
            uint32_t                           FirstSubresource,
            std::span<const SubresourceDesc>   Subresources,
            std::optional<RHI::MResourceState> TransitionState) override;

        /// <summary>
        /// Get the underlying D3D12 resource.
        /// </summary>
        [[nodiscard]] ID3D12Resource* GetResource() const;

        /// <summary>
        /// Get the underlying D3D12 resource allocation.
        /// </summary>
        [[nodiscard]] D3D12MA::Allocation* GetAllocation() const;

        /// <summary>
        /// Release the underlying D3D12 resource without enqueueing a delete.
        /// </summary>
        void SilentRelease();

    public:
        GpuResourceHandle GetHandle(
            size_t Offset) const override;

        uint8_t* Map() override;

        void Unmap() override;

    private:
        /// <summary>
        /// Initialize the texture description.
        /// </summary>
        void InitializeDesc();

        /// <summary>
        /// Get the size of the texture in bytes to copy.
        /// </summary>
        [[nodiscard]] size_t GetTextureCopySize(
            uint32_t SubresourcesCount);

    protected:
        WinAPI::ComPtr<ID3D12Resource>      m_Resource;
        WinAPI::ComPtr<D3D12MA::Allocation> m_Allocation;
        ClearOperationOpt                   m_ClearValue;
    };
} // namespace Neon::RHI