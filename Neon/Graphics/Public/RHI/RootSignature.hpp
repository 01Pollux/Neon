#pragma once

#include <Core/Neon.hpp>
#include <RHI/Resource/View.hpp>
#include <Math/Common.hpp>

#include <list>
#include <vector>
#include <variant>

#include <boost/serialization/list.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>

namespace Neon::RHI
{
    namespace RSCommon
    {
        enum class Type : uint8_t
        {
            Material,
            BlurPass,
            CopyToTexture,
            GridFrustum,
            LightCull,

#ifndef NEON_DIST
            DebugLine,
#endif

            _Count
        };

        //

        enum class MaterialRS : uint8_t
        {
            LocalData,
            SharedData,
            InstanceData,

            LightData,
            FrameData,

            //

            _SamplersStart,

            g_Samplers = _SamplersStart,
            g_SamplersCmpState,

            _SamplersEnd = g_SamplersCmpState,

            //

            _ResourcesStart,

            g_RW_Buffer = _ResourcesStart,
            g_Buffer,

            g_RW_Texture1D_1f,
            g_RW_Texture1D_2f,
            g_RW_Texture1D_3f,
            g_RW_Texture1D_4f,

            g_RW_Texture1D_1i,
            g_RW_Texture1D_2i,
            g_RW_Texture1D_3i,
            g_RW_Texture1D_4i,

            g_RW_Texture1D_1u,
            g_RW_Texture1D_2u,
            g_RW_Texture1D_3u,
            g_RW_Texture1D_4u,

            g_Texture1D,

            g_RW_Texture2D_1f,
            g_RW_Texture2D_2f,
            g_RW_Texture2D_3f,
            g_RW_Texture2D_4f,

            g_RW_Texture2D_1i,
            g_RW_Texture2D_2i,
            g_RW_Texture2D_3i,
            g_RW_Texture2D_4i,

            g_RW_Texture2D_1u,
            g_RW_Texture2D_2u,
            g_RW_Texture2D_3u,
            g_RW_Texture2D_4u,

            g_Texture2D,

            g_Texture2DMS_1f,
            g_Texture2DMS_2f,
            g_Texture2DMS_3f,
            g_Texture2DMS_4f,

            g_Texture2DMS_1i,
            g_Texture2DMS_2i,
            g_Texture2DMS_3i,
            g_Texture2DMS_4i,

            g_Texture2DMS_1u,
            g_Texture2DMS_2u,
            g_Texture2DMS_3u,
            g_Texture2DMS_4u,

            g_RW_Texture3D_1f,
            g_RW_Texture3D_2f,
            g_RW_Texture3D_3f,
            g_RW_Texture3D_4f,

            g_RW_Texture3D_1i,
            g_RW_Texture3D_2i,
            g_RW_Texture3D_3i,
            g_RW_Texture3D_4i,

            g_RW_Texture3D_1u,
            g_RW_Texture3D_2u,
            g_RW_Texture3D_3u,
            g_RW_Texture3D_4u,

            g_Texture3D,
            g_TextureCube,

            _ResourcesEnd = g_TextureCube
        };
    } // namespace RSCommon

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
        /// <summary>
        /// Add srv descriptor entries
        /// </summary>
        RootDescriptorTable& AddSrvRange(
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddSrvRangeAt(
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
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add uav descriptor entries
        /// </summary>
        RootDescriptorTable& AddUavRange(
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddUavRangeAt(
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
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add cbv descriptor entries
        /// </summary>
        RootDescriptorTable& AddCbvRange(
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddCbvRangeAt(
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
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            uint32_t                  Offset,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Data_Static_While_Execute, ERootDescriptorTableFlags::Descriptor_Volatile));

        /// <summary>
        /// Add sampler descriptor entries
        /// </summary>
        RootDescriptorTable& AddSamplerRange(
            uint32_t                  BaseShaderRegister,
            uint32_t                  RegisterSpace,
            uint32_t                  NumDescriptors,
            MRootDescriptorTableFlags Flags = BitMask_Or(ERootDescriptorTableFlags::Descriptor_Volatile))
        {
            return AddSamplerRangeAt(
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
        std::vector<RootDescriptorTableParam> m_DescriptorRanges;
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
            return Add32BitConstants(ShaderRegister, RegisterSpace, uint32_t(Math::DivideByMultiple(sizeof(_Ty), sizeof(uint32_t))), Visibility);
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
            uint32_t             ShaderRegister,
            uint32_t             RegisterSpace,
            ShaderVisibility     Visibility = ShaderVisibility::All,
            MRootDescriptorFlags Flags      = {});

        /// <summary>
        /// Add srv root
        /// </summary>
        RootSignatureBuilder& AddShaderResourceView(
            uint32_t             ShaderRegister,
            uint32_t             RegisterSpace,
            ShaderVisibility     Visibility = ShaderVisibility::All,
            MRootDescriptorFlags Flags      = {});

        /// <summary>
        /// Add uav root
        /// </summary>
        RootSignatureBuilder& AddUnorderedAccessView(
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
        String                         m_Name;
        std::vector<RootParameter>     m_Parameters;
        std::vector<StaticSamplerDesc> m_StaticSamplers;
        MRootSignatureBuilderFlags     m_Flags;
    };

    class IRootSignature
    {
    public:
        using CommonRootsignatureList = std::array<Ptr<IRootSignature>, size_t(RSCommon::Type::_Count)>;

    public:
        [[nodiscard]] static Ptr<IRootSignature> Create(
            const RootSignatureBuilder& Builder);

        [[nodiscard]] static Ptr<IRootSignature> Get(
            RSCommon::Type Type);

        /// <summary>
        /// Load common root signatures
        /// Must be called once in the application
        /// </summary>
        [[nodiscard]] static CommonRootsignatureList Load();

        virtual ~IRootSignature() = default;
    };
} // namespace Neon::RHI