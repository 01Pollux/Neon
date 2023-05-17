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
    class InputLayoutDesc
    {
    public:
        InputLayoutDesc() = default;

        InputLayoutDesc(size_t ElementsCount) :
            m_Elements(ElementsCount, D3D12_INPUT_ELEMENT_DESC{}),
            m_SemanticNames(ElementsCount)
        {
        }

        void AppendElement(
            const D3D12_INPUT_ELEMENT_DESC& Element)
        {
            m_SemanticNames.emplace_back(Element.SemanticName);
            m_Elements.emplace_back(Element).SemanticName = m_SemanticNames.back().c_str();
        }

        void SetElement(
            size_t                          Index,
            const D3D12_INPUT_ELEMENT_DESC& Element)
        {
            auto& SemanticName = *std::next(m_SemanticNames.begin(), Index);
            SemanticName       = Element.SemanticName;

            m_Elements[Index]              = Element;
            m_Elements[Index].SemanticName = SemanticName.c_str();
        }

        void Resize(
            size_t Size)
        {
            m_Elements.resize(Size, D3D12_INPUT_ELEMENT_DESC{});
            m_SemanticNames.resize(Size);
        }

        void Clear()
        {
            m_Elements.clear();
            m_SemanticNames.clear();
        }

        [[nodiscard]] D3D12_INPUT_LAYOUT_DESC GetElementDesc() const
        {
            D3D12_INPUT_LAYOUT_DESC Layout{
                .pInputElementDescs = m_Elements.data(),
                .NumElements        = uint32_t(m_Elements.size())
            };
            return Layout;
        }

        [[nodiscard]] InputLayoutDesc Clone() const
        {
            InputLayoutDesc Desc(m_Elements.size());
            for (size_t i = 0; i < m_Elements.size(); i++)
            {
                Desc.SetElement(i, m_Elements[i]);
            }
            return Desc;
        }

    private:
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_Elements;
        std::list<StringU8>                   m_SemanticNames;
    };

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
