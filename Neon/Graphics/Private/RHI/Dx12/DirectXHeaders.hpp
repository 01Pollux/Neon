//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include <Core/String.hpp>

#include <DX/d3d12.h>
#include <DX/d3dx12.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>

#include <Window/Win32/WinPtr.hpp>
#include <Private/RHI/Dx12/D3D12MemAlloc.hpp>

namespace DX = DirectX;

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

namespace Neon::RHI
{
    void ThrowIfFailed(
        HRESULT Result);

    inline void RenameObject(
        ID3D12Object*  Resource,
        const wchar_t* Name)
    {
#if !NEON_DIST
        Resource->SetName(Name);
#endif
    }

    inline void RenameObject(
        ID3D12Object*  Resource,
        const wchar_t* Name,
        size_t         Index)
    {
#if !NEON_DIST
        RenameObject(Resource, StringUtils::Format(Name, Index).c_str());
#endif
    }

    template<typename _Ty>
        requires std::conjunction_v<std::is_base_of<ID3D12Object, _Ty>,
                                    std::negation<std::is_same<ID3D12Object, _Ty>>>
    inline void RenameObject(
        const Win32::ComPtr<_Ty>& Resource,
        const wchar_t*            Name)
    {
        RenameObject(Resource.Get(), Name);
    }

    template<typename _Ty>
        requires std::conjunction_v<std::is_base_of<ID3D12Object, _Ty>,
                                    std::negation<std::is_same<ID3D12Object, _Ty>>>
    inline void RenameObject(
        const Win32::ComPtr<_Ty>& Resource,
        const wchar_t*            Name,
        size_t                    Index)
    {
        RenameObject(Resource.Get(), Name, Index);
    }

    // Helper which applies one or more resources barriers and then reverses them on destruction.
    template<size_t _Size>
    class ScopedBarrierS
    {
    public:
        ScopedBarrierS(
            ID3D12GraphicsCommandList*                       CommandList,
            const std::array<D3D12_RESOURCE_BARRIER, _Size>& Barriers) noexcept(false) :
            m_CommandList(CommandList),
            m_Barriers(Barriers)
        {
            // Set barriers
            m_CommandList->ResourceBarrier(UINT(m_Barriers.size()), m_Barriers.data());
        }

        ScopedBarrierS(ScopedBarrierS&&)            = delete;
        ScopedBarrierS& operator=(ScopedBarrierS&&) = delete;

        ScopedBarrierS(ScopedBarrierS const&)            = delete;
        ScopedBarrierS& operator=(ScopedBarrierS const&) = delete;

        ~ScopedBarrierS()
        {
            // reverse barrier inputs and outputs
            for (auto& Barrier : m_Barriers)
            {
                std::swap(Barrier.Transition.StateAfter, Barrier.Transition.StateBefore);
            }

            // Set barriers
            m_CommandList->ResourceBarrier(UINT(m_Barriers.size()), m_Barriers.data());
        }

    private:
        ID3D12GraphicsCommandList*                m_CommandList;
        std::array<D3D12_RESOURCE_BARRIER, _Size> m_Barriers;
    };

    // Helper which applies one or more resources barriers and then reverses them on destruction.
    class ScopedBarrierD
    {
    public:
        ScopedBarrierD(
            _In_ ID3D12GraphicsCommandList*               CommandList,
            std::initializer_list<D3D12_RESOURCE_BARRIER> Barriers) noexcept(false) :
            m_CommandList(CommandList),
            m_Barriers(Barriers)
        {
            // Set barriers
            m_CommandList->ResourceBarrier(UINT(m_Barriers.size()), m_Barriers.data());
        }

        ScopedBarrierD(
            _In_ ID3D12GraphicsCommandList*                 CommandList,
            _In_reads_(count) const D3D12_RESOURCE_BARRIER* Barriers,
            size_t                                          Count) noexcept(false) :
            m_CommandList(CommandList),
            m_Barriers(Barriers, Barriers + Count)
        {
            // Set barriers
            m_CommandList->ResourceBarrier(UINT(m_Barriers.size()), m_Barriers.data());
        }

        template<size_t _Size>
        ScopedBarrierD(
            _In_ ID3D12GraphicsCommandList* CommandList,
            const D3D12_RESOURCE_BARRIER (&Barriers)[_Size]) noexcept(false) :
            m_CommandList(CommandList),
            m_Barriers(Barriers, Barriers + _Size)
        {
            // Set barriers
            m_CommandList->ResourceBarrier(UINT(m_Barriers.size()), m_Barriers.data());
        }

        ScopedBarrierD(ScopedBarrierD&& Other)            = default;
        ScopedBarrierD& operator=(ScopedBarrierD&& Other) = default;

        ScopedBarrierD(ScopedBarrierD const&)            = delete;
        ScopedBarrierD& operator=(ScopedBarrierD const&) = delete;

        ~ScopedBarrierD()
        {
            // reverse barrier inputs and outputs
            for (auto& Barrier : m_Barriers)
            {
                std::swap(Barrier.Transition.StateAfter, Barrier.Transition.StateBefore);
            }

            // Set barriers
            m_CommandList->ResourceBarrier(UINT(m_Barriers.size()), m_Barriers.data());
        }

    private:
        ID3D12GraphicsCommandList*          m_CommandList;
        std::vector<D3D12_RESOURCE_BARRIER> m_Barriers;
    };

} // namespace Neon::RHI
