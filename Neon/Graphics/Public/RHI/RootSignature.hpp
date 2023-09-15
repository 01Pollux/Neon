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
    /**
     * To allow dynamic material system to manage any kind of resources and batch descriptors together
     * We need to have a way to describe the root signature of a material aswell as take some compromises
     *
     * - A root signature can query a descriptor table, a constant buffer or a root descriptor by name
     * - A root signature can reserve the space needed for its entries individually so we can seperate local and shared resources across materials
     * - Using shared resources will allow us to batch draw calls together, meaning one must know in advance the indices of the resources that'll be used,
     * to allow such thing, we need to also insert a structured buffer that'll contain the indices of the resources used by the material
     *
     * * Suppose we have a material that does the following:
     * - Uses a buffer named "PerFrame" as a constant buffer (vertex + pixel)
     * - Uses a group of textures named "Textures[]" of size 4 as a shader resource, these are instanced (pixel)
     * - Uses a structured buffer named "Buffer", this is also instanced (vertex + pixel)
     *
     * Normally to resolve such thing, we'd need to have a root signature that looks like this:
     * - | CBV [1] (b0, s1)      | SRV [1] (t0, s1) INST |  <--- PerFrame + Buffer
     * - | SRV [4] (t0, s2) INST |                          <--- Textures
     *
     * Internally, it will be transformed into this:
     * - | CBV [1] (b0, s1) | SRV [] (t0, s1) | <--- PerFrame + Buffer
     * - | SRV [] (t0, s2)  |                   <--- Textures
     *
     */

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
        bool                      Instanced;

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
            Archive& Instanced;
        }
    };

    class RootDescriptorTable
    {
    public:
        /// <summary>
        /// Add srv descriptor entries
        /// </summary>
        RootDescriptorTable& AddSrvRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddSrvRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Instanced,
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
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add uav descriptor entries
        /// </summary>
        RootDescriptorTable& AddUavRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddUavRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Instanced,
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
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add cbv descriptor entries
        /// </summary>
        RootDescriptorTable& AddCbvRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddCbvRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Instanced,
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
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add sampler descriptor entries
        /// </summary>
        RootDescriptorTable& AddSamplerRange(
            StringU8                  Name,
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddSamplerRangeAt(
                std::move(Name),
                BaseShaderRegister,
                RegisterSpace,
                NumDescriptors,
                std::numeric_limits<uint32_t>::max(),
                Instanced,
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
            bool                      Instanced = false,
            MRootDescriptorTableFlags Flags     = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

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

    private:
        friend class boost::serialization::access;
        template<typename _Archive>
        void serialize(
            _Archive& Archive,
            uint32_t  Version)
        {
            Archive& m_DescriptorRanges;
        }

    private:
        std::list<std::pair<StringU8, RootDescriptorTableParam>> m_DescriptorRanges;
    };

    class RootParameter
    {
    public:
        using DescriptorTable = RootDescriptorTable;

        struct Constants
        {
            StringU8 Name;
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
                Archive& Name;
                Archive& ShaderRegister;
                Archive& Num32BitValues;
                Archive& RegisterSpace;
            }
        };

        using RootType = DescriptorTableParam;
        struct Root
        {
            StringU8             Name;
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
                Archive& Name;
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
            return Add32BitConstants(std::move(Name), ShaderRegister, RegisterSpace, sizeof(_Ty) / 4, Visibility);
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
            StringU8                 Name,
            const StaticSamplerDesc& Desc);

        /// <summary>
        /// Add standard static sampler
        /// </summary>
        RootSignatureBuilder& AddStandardSamplers(
            uint32_t         RegisterSpace = 0,
            ShaderVisibility Visibility    = ShaderVisibility::Pixel);

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
        /// Remove parameter from root signature
        /// </summary>
        void RemoveParameter(
            const StringU8& Name);

        /// <summary>
        /// Remove static sampler from root signature
        /// </summary>
        void RemoveStaticSampler(
            const StringU8& Name);

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
        String                                            m_Name;
        std::list<RootParameter>                          m_Parameters;
        std::list<std::pair<StringU8, StaticSamplerDesc>> m_StaticSamplers;
        MRootSignatureBuilderFlags                        m_Flags;
    };

    class IRootSignature
    {
    public:
        [[nodiscard]] static Ptr<IRootSignature> Create(
            const RootSignatureBuilder& Builder);

        virtual ~IRootSignature() = default;

        enum class ParamType : uint8_t
        {
            Constants,
            Root,
            DescriptorTable
        };

        struct QueryResult
        {
            StringU8 Name;
            uint32_t RootIndex;
            union {
                struct
                {
                    uint8_t Num32BitValues;
                } Constants;
                struct
                {
                    RootParameter::RootType Type;
                } Root;
                struct
                {
                    uint32_t             Size;
                    DescriptorTableParam Type;
                    bool                 Instanced;
                } Descriptor;
            };
            ParamType Type;
        };

        /// <summary>
        /// Query parameter by name
        /// </summary>
        [[nodiscard]] std::optional<const QueryResult*> QueryParam(
            const StringU8& Name) const
        {
            auto Iter = m_ParamMap.find(Name);
            return Iter != m_ParamMap.end() ? std::make_optional(&m_Params[Iter->second]) : std::nullopt;
        }

        /// <summary>
        /// Get all parameters of root signature
        /// </summary>
        [[nodiscard]] const auto& GetParams() const
        {
            return m_Params;
        }

        /// <summary>
        /// Get all parameters of root signature in map
        /// </summary>
        [[nodiscard]] const auto& GetParamMap() const
        {
            return m_ParamMap;
        }

        /// <summary>
        /// Get parameter by name else return nullptr
        /// </summary>
        [[nodiscard]] const auto GetParam(
            const StringU8& Name) const
        {
            auto Iter = m_ParamMap.find(Name);
            return Iter != m_ParamMap.end() ? &m_Params[Iter->second] : nullptr;
        }

    protected:
        std::vector<QueryResult>             m_Params;
        std::unordered_map<StringU8, size_t> m_ParamMap;
    };
} // namespace Neon::RHI