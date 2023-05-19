#pragma once

#include <RHI/Resource/View.hpp>
#include <Core/Neon.hpp>
#include <vector>
#include <variant>

namespace Neon::RHI
{
    enum class ShaderVisibility : uint8_t
    {
        All,
        Vertex,
        Hull,
        Domain,
        Geometry,
        Pixel,
    };

    enum class DescriptorTableParam : uint8_t
    {
        ConstantBuffer,
        ShaderResource,
        UnorderedAccess,
        Sampler,
    };

    struct StaticSamplerDesc : SamplerDesc
    {
        uint32_t         ShaderRegister;
        uint32_t         RegisterSpace;
        ShaderVisibility Visibility = ShaderVisibility::Pixel;
    };

    struct RootDescriptorTableParam
    {
        uint32_t             ShaderRegister;
        uint32_t             RegisterSpace;
        uint32_t             DescriptorCount;
        MRootDescriptorFlags Flags;
        DescriptorTableParam Type;
    };

    class RootDescriptorTable
    {
    public:
        RootDescriptorTable() = default;
        RootDescriptorTable(
            size_t Reserve);

        /// <summary>
        /// Add srv descriptor entries
        /// </summary>
        RootDescriptorTable& AddSrvRange(
            uint32_t             BaseShaderRegister,
            uint32_t             RegisterSpace,
            uint32_t             NumDescriptors,
            MRootDescriptorFlags Flags = MRootDescriptorFlags::FromEnum(ERootDescriptorFlags::Data_Volatile));

        /// <summary>
        /// Add uav descriptor entries
        /// </summary>
        RootDescriptorTable& AddUavRange(
            uint32_t             BaseShaderRegister,
            uint32_t             RegisterSpace,
            uint32_t             NumDescriptors,
            MRootDescriptorFlags Flags = MRootDescriptorFlags::FromEnum(ERootDescriptorFlags::Data_Volatile));

        /// <summary>
        /// Add cbv descriptor entries
        /// </summary>
        RootDescriptorTable& AddCbvRange(
            uint32_t             BaseShaderRegister,
            uint32_t             RegisterSpace,
            uint32_t             NumDescriptors,
            MRootDescriptorFlags Flags = MRootDescriptorFlags::FromEnum(ERootDescriptorFlags::Data_Static_While_Execute));

        /// <summary>
        /// Add sampler descriptor entries
        /// </summary>
        RootDescriptorTable& AddSamplerRange(
            uint32_t             BaseShaderRegister,
            uint32_t             RegisterSpace,
            uint32_t             NumDescriptors,
            MRootDescriptorFlags Flags = {});

    private:
        std::vector<RootDescriptorTableParam> m_DescriptorRanges;
    };

    class RootParameter
    {
    public:
        struct Constants
        {
            uint32_t ShaderRegister;
            uint32_t Num32BitValues;
            uint32_t RegisterSpace;
        };

        using DescriptorType = DescriptorTableParam;
        struct Descriptor
        {

            uint32_t       ShaderRegister;
            uint32_t       RegisterSpace;
            DescriptorType Type;
        };

        using Variant = std::variant<RootDescriptorTable, Constants, Descriptor>;

        RootParameter(
            Variant          Param,
            ShaderVisibility Visibility);

    private:
        Variant          m_Parameter;
        ShaderVisibility m_Visibility;
    };

    enum class ERootSignatureBuilderFlags : uint8_t
    {
        AllowInputLayout,
        DenyVSAccess,
        DenyHSAccess,
        DenyDSAccess,
        DenyGSAccess,
        DenyPSAccess,
    };
    using MRootSignatureBuilderFlags = Bitmask<ERootSignatureBuilderFlags>;

    class RootSignatureBuilder
    {
    public:
        /// <summary>
        /// Add descriptor table
        /// </summary>
        RootSignatureBuilder& AddDescriptorTable(
            RootDescriptorTable Table,
            ShaderVisibility    Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add 32 bit root constant
        /// </summary>
        template<typename _Ty>
        RootSignatureBuilder& Add32BitConstants(
            uint32_t         ShaderRegister,
            uint32_t         RegisterSpace,
            ShaderVisibility Visibility = ShaderVisibility::All)
        {
            static_assert(sizeof(_Ty) % 4 == 0);
            return Add32BitConstants(ShaderRegister, RegisterSpace, sizeof(_Ty) / 4, Visibility);
        }

        /// <summary>
        /// Add 32 bit root constants
        /// </summary>
        RootSignatureBuilder& Add32BitConstants(
            uint32_t         ShaderRegister,
            uint32_t         RegisterSpace,
            uint32_t         Num32BitValues,
            ShaderVisibility Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add cbv root
        /// </summary>
        RootSignatureBuilder& AddConstantBufferView(
            uint32_t         ShaderRegister,
            uint32_t         RegisterSpace,
            uint32_t         Num32BitValues,
            ShaderVisibility Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add srv root
        /// </summary>
        RootSignatureBuilder& AddShaderResourceView(
            uint32_t         ShaderRegister,
            uint32_t         RegisterSpace,
            ShaderVisibility Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add uav root
        /// </summary>
        RootSignatureBuilder& AddUnorderedAccessView(
            uint32_t         ShaderRegister,
            uint32_t         RegisterSpace,
            ShaderVisibility Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add uav with counter root
        /// </summary>
        RootSignatureBuilder& AddUnorderedAccessViewWithCounter(
            uint32_t             ShaderRegister,
            uint32_t             RegisterSpace,
            MRootDescriptorFlags Flags      = {},
            ShaderVisibility     Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add static sampler
        /// </summary>
        RootSignatureBuilder& AddSampler(
            const StaticSamplerDesc& Desc);

        /// <summary>
        /// Set flags for root signature
        /// </summary>
        RootSignatureBuilder& SetFlags(
            ERootSignatureBuilderFlags Flag,
            bool                       Value = true);

    private:
        std::vector<RootParameter>     m_Parameters;
        std::vector<StaticSamplerDesc> m_StaticSamplers;
        MRootSignatureBuilderFlags     m_Flags;
    };

    class IRootSignature
    {
    public:
        [[nodiscard]] Ptr<IRootSignature> Create(
            const RootSignatureBuilder& Builder);

        virtual ~IRootSignature() = default;

        /// <summary>
        /// Get resource count
        /// </summary>
        [[nodiscard]] virtual uint32_t GetResourceCount() = 0;

        /// <summary>
        /// Get sampler count
        /// </summary>
        [[nodiscard]] virtual uint32_t GetSamplerCount() = 0;
    };
} // namespace Neon::RHI