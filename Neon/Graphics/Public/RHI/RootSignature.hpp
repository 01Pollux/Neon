#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/View.hpp>

#include <list>
#include <vector>
#include <variant>

#include <boost/serialization/list.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>

namespace Neon::RHI
{
    class IRootSignature;

    enum class ERootDescriptorTableFlags : uint8_t
    {
        Descriptor_Volatile,
        Descriptor_Static_Bounds_Check,
        Data_Volatile,
        Data_Static_While_Execute,
        Data_Static,

        _Last_Enum
    };
    using MRootDescriptorTableFlags = Bitmask<ERootDescriptorTableFlags>;

    enum class ERootDescriptorFlags : uint8_t
    {
        Data_Volatile,
        Data_Static_While_Execute,
        Data_Static,

        _Last_Enum
    };
    using MRootDescriptorFlags = Bitmask<ERootDescriptorFlags>;

    //

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
        uint32_t                  ShaderRegister;
        uint32_t                  RegisterSpace;
        uint32_t                  DescriptorCount;
        uint32_t                  Offset = std::numeric_limits<uint32_t>::max();
        MRootDescriptorTableFlags Flags;
        DescriptorTableParam      Type;

        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t)
        {
            Archive& ShaderRegister;
            Archive& RegisterSpace;
            Archive& DescriptorCount;
            Archive& Offset;
            Archive& Flags;
            auto     DType = std::to_underlying(Type);
            Archive& DType;
            Type = static_cast<Neon::RHI::DescriptorTableParam>(DType);
        }
    };

    class RootDescriptorTable
    {
    public:
        RootDescriptorTable(
            bool Instanced = false) :
            m_Instanced(Instanced)
        {
        }

        /// <summary>
        /// Add srv descriptor entries
        /// </summary>
        RootDescriptorTable& AddSrvRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddSrvRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Flags);
        }

        /// <summary>
        /// Add srv descriptor entries
        /// </summary>
        RootDescriptorTable& AddSrvRangeAt(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add uav descriptor entries
        /// </summary>
        RootDescriptorTable& AddUavRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddUavRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Flags);
        }

        /// <summary>
        /// Add uav descriptor entries
        /// </summary>
        RootDescriptorTable& AddUavRangeAt(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add cbv descriptor entries
        /// </summary>
        RootDescriptorTable& AddCbvRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddCbvRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Flags);
        }

        /// <summary>
        /// Add cbv descriptor entries
        /// </summary>
        RootDescriptorTable& AddCbvRangeAt(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add sampler descriptor entries
        /// </summary>
        RootDescriptorTable& AddSamplerRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddSamplerRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Flags);
        }

        /// <summary>
        /// Add sampler descriptor entries
        /// </summary>
        RootDescriptorTable& AddSamplerRangeAt(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Descriptor_Volatile));

    public:
        /// <summary>
        /// Get the descriptor ranges of this table
        /// </summary>
        [[nodiscard]] const auto& GetRanges() const noexcept
        {
            return m_DescriptorRanges;
        }

        /// <summary>
        /// Get the descriptor ranges of this table
        /// </summary>
        [[nodiscard]] auto& GetRanges() noexcept
        {
            return m_DescriptorRanges;
        }

        /// <summary>
        /// Check to see if the table is instanced
        /// </summary>
        [[nodiscard]] bool Instanced() const noexcept
        {
            return m_Instanced;
        }

    private:
        friend class boost::serialization::access;
        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t  Version)
        {
            Archive& m_DescriptorRanges;
            Archive& m_Instanced;
        }

    private:
        std::vector<
            std::pair<StringU8, RootDescriptorTableParam>>
             m_DescriptorRanges;
        bool m_Instanced = false;
    };

    class RootParameter
    {
    public:
        using DescriptorTable = RootDescriptorTable;

        struct Constants
        {
            uint32_t ShaderRegister;
            uint32_t RegisterSpace;
            uint32_t Num32BitValues;

        private:
            friend class boost::serialization::access;
            template<typename _Archive>
            void serialize(
                _Archive& Archive,
                uint32_t)
            {
                Archive& ShaderRegister;
                Archive& Num32BitValues;
                Archive& RegisterSpace;
            }
        };

        using RootType = DescriptorTableParam;
        struct Root
        {
            uint32_t             ShaderRegister;
            uint32_t             RegisterSpace;
            RootType             Type;
            MRootDescriptorFlags Flags;

        private:
            friend class boost::serialization::access;
            template<typename _Archive>
            void serialize(
                _Archive& Archive,
                uint32_t)
            {
                Archive& ShaderRegister;
                Archive& RegisterSpace;
                auto     DType = std::to_underlying(Type);
                Archive& DType;
                Type = static_cast<RootType>(DType);
                Archive& Flags;
            }
        };

        using Variant = boost::variant<DescriptorTable, Constants, Root>;

        RootParameter() = default;

        RootParameter(
            Variant          Param,
            ShaderVisibility Visibility);

        /// <summary>
        /// Get the parameter variant
        /// </summary>
        [[nodiscard]] const auto& GetParameter() const noexcept
        {
            return m_Parameter;
        }

        /// <summary>
        /// Get the parameter variant
        /// </summary>
        [[nodiscard]] auto& GetParameter() noexcept
        {
            return m_Parameter;
        }

        /// <summary>
        /// Get the visibility of this parameter
        /// </summary>
        [[nodiscard]] auto Visibility() const noexcept
        {
            return m_Visibility;
        }

        /// <summary>
        /// Get the visibility of this parameter
        /// </summary>
        auto& Visibility(
            ShaderVisibility Visibility) noexcept
        {
            m_Visibility = Visibility;
            return *this;
        }

    private:
        friend class boost::serialization::access;
        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t)
        {
            Archive& m_Parameter;
            auto     Visibility = std::to_underlying(m_Visibility);
            Archive& Visibility;
            m_Visibility = static_cast<ShaderVisibility>(Visibility);
        }

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
        AllowStreamOutput,
        LocalRootSignature,
        DenyAmpAccess,
        DenyMeshAccess,

        _Last_Enum
    };
    using MRootSignatureBuilderFlags = Bitmask<ERootSignatureBuilderFlags>;

    class RootSignatureBuilder
    {
    public:
        RootSignatureBuilder(
            String Name = STR("")) noexcept :
            m_Name(std::move(Name))
        {
        }

        /// <summary>
        /// Set name of root signature
        /// </summary>
        RootSignatureBuilder& SetName(
            String Name) noexcept
        {
            m_Name = std::move(Name);
            return *this;
        }

        /// <summary>
        /// Get name of root signature
        /// </summary>
        [[nodiscard]] const String& GetName() const noexcept
        {
            return m_Name;
        }

        /// <summary>
        /// Add descriptor table
        /// </summary>
        RootSignatureBuilder& AddDescriptorTable(
            StringU8            Name,
            RootDescriptorTable Table,
            ShaderVisibility    Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add 32 bit root constant
        /// </summary>
        template<typename _Ty>
        RootSignatureBuilder& Add32BitConstants(
            StringU8         Name,
            uint32_t         ShaderRegister,
            uint32_t         RegisterSpace,
            ShaderVisibility Visibility = ShaderVisibility::All)
        {
            static_assert(sizeof(_Ty) % 4 == 0);
            return Add32BitConstants(Name, ShaderRegister, RegisterSpace, sizeof(_Ty) / 4, Visibility);
        }

        /// <summary>
        /// Add 32 bit root constants
        /// </summary>
        RootSignatureBuilder& Add32BitConstants(
            StringU8         Name,
            uint32_t         ShaderRegister,
            uint32_t         RegisterSpace,
            uint32_t         Num32BitValues,
            ShaderVisibility Visibility = ShaderVisibility::All);

        /// <summary>
        /// Add cbv root
        /// </summary>
        RootSignatureBuilder& AddConstantBufferView(
            StringU8             Name,
            uint32_t             ShaderRegister,
            uint32_t             RegisterSpace,
            ShaderVisibility     Visibility = ShaderVisibility::All,
            MRootDescriptorFlags Flags      = {});

        /// <summary>
        /// Add srv root
        /// </summary>
        RootSignatureBuilder& AddShaderResourceView(
            StringU8             Name,
            uint32_t             ShaderRegister,
            uint32_t             RegisterSpace,
            ShaderVisibility     Visibility = ShaderVisibility::All,
            MRootDescriptorFlags Flags      = {});

        /// <summary>
        /// Add uav root
        /// </summary>
        RootSignatureBuilder& AddUnorderedAccessView(
            StringU8             Name,
            uint32_t             ShaderRegister,
            uint32_t             RegisterSpace,
            ShaderVisibility     Visibility = ShaderVisibility::All,
            MRootDescriptorFlags Flags      = {});

        /// <summary>
        /// Add static sampler
        /// </summary>
        RootSignatureBuilder& AddSampler(
            const StaticSamplerDesc& Desc);

        /// <summary>
        /// Add standard static sampler
        /// </summary>
        RootSignatureBuilder& AddStandardSamplers(
            uint32_t         RegisterSpace = 0,
            ShaderVisibility Visibility    = ShaderVisibility::All);

        /// <summary>
        /// Set flags for root signature
        /// </summary>
        RootSignatureBuilder& SetFlags(
            ERootSignatureBuilderFlags Flag,
            bool                       Value = true);

        /// <summary>
        /// Set flags for root signature to only use compute shader only
        /// </summary>
        template<ERootSignatureBuilderFlags... Flags>
        RootSignatureBuilder& ComputeOnly(
            bool Value = true)
        {
            SetFlags(RHI::ERootSignatureBuilderFlags::AllowInputLayout, false);
            SetFlags(RHI::ERootSignatureBuilderFlags::AllowStreamOutput, false);
            SetFlags(RHI::ERootSignatureBuilderFlags::LocalRootSignature, false);
            SetFlags(RHI::ERootSignatureBuilderFlags::DenyVSAccess);
            SetFlags(RHI::ERootSignatureBuilderFlags::DenyHSAccess);
            SetFlags(RHI::ERootSignatureBuilderFlags::DenyDSAccess);
            SetFlags(RHI::ERootSignatureBuilderFlags::DenyGSAccess);
            SetFlags(RHI::ERootSignatureBuilderFlags::DenyPSAccess);
            SetFlags(RHI::ERootSignatureBuilderFlags::DenyAmpAccess);
            SetFlags(RHI::ERootSignatureBuilderFlags::DenyMeshAccess);
            return *this;
        }

    public:
        /// <summary>
        /// Get parameters of root signature
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] const auto& GetParameters() const noexcept
        {
            return m_Parameters;
        }

        /// <summary>
        /// Get static samplers of root signature
        /// </summary>
        [[nodiscard]] const auto& GetSamplers() const noexcept
        {
            return m_StaticSamplers;
        }

        /// <summary>
        /// Get flags of root signature
        /// </summary>
        [[nodiscard]] const auto& GetFlags() const noexcept
        {
            return m_Flags;
        }

        /// <summary>
        /// Build root signature from builder
        /// </summary>
        [[nodiscard]] Ptr<IRootSignature> Build() const;

    private:
        friend class boost::serialization::access;
        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t  Version)
        {
            Archive& m_Parameters;
            Archive& m_StaticSamplers;
            Archive& m_Flags;
        }

    private:
        String                                          m_Name;
        std::vector<std::pair<StringU8, RootParameter>> m_Parameters;
        std::vector<StaticSamplerDesc>                  m_StaticSamplers;
        MRootSignatureBuilderFlags                      m_Flags;
    };

    class IRootSignature
    {
    public:
        struct ParamRef
        {
            StringU8 Name;
        };

        struct ParamDescriptorRange : ParamRef
        {
            uint32_t             Offset;
            uint32_t             Size;
            DescriptorTableParam Type;
        };

        struct ParamConstant : ParamRef
        {
            uint8_t Num32BitValues;
        };

        struct ParamRoot : ParamRef
        {
            RootParameter::RootType Type;
        };

        struct ParamDescriptor : ParamRef
        {
            std::unordered_map<StringU8, ParamDescriptorRange> NamedRanges;

            uint32_t Size;
            bool     Instanced;

            /// <summary>
            /// Get parameter by name else return nullptr
            /// </summary>
            [[nodiscard]] const ParamDescriptorRange* FindRange(
                const StringU8& Name) const
            {
                auto Iter = NamedRanges.find(Name);
                return Iter != NamedRanges.end() ? &Iter->second : nullptr;
            }
        };

        using ParamInfo = boost::variant<ParamConstant, ParamRoot, ParamDescriptor>;

    public:
        [[nodiscard]] static Ptr<IRootSignature> Create(
            const RootSignatureBuilder& Builder);

        virtual ~IRootSignature() = default;

        /// <summary>
        /// Get all parameters of root signature ordered by root index
        /// </summary>
        [[nodiscard]] const auto& GetParams() const
        {
            return m_Params;
        }

        /// <summary>
        /// Get all parameters of root signature in map
        /// </summary>
        [[nodiscard]] const auto& GetNamedParams() const
        {
            return m_NamedParams;
        }

        /// <summary>
        /// Get parameter by name else return nullptr
        /// </summary>
        [[nodiscard]] const ParamInfo* FindParam(
            const StringU8& Name) const
        {
            auto Iter = m_NamedParams.find(Name);
            return Iter != m_NamedParams.end() ? &m_Params[Iter->second] : nullptr;
        }

    protected:
        std::vector<ParamInfo>                 m_Params;
        std::unordered_map<StringU8, uint32_t> m_NamedParams;
    };
} // namespace Neon::RHI