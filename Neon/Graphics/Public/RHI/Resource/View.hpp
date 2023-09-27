#pragma once

#include <RHI/Resource/Common.hpp>
#include <variant>
#include <array>

namespace Neon::RHI
{
    struct ShaderComponentMapping
    {
    public:
        static constexpr uint32_t c_ShiftCount = 3;
        static constexpr uint32_t c_ShiftMask  = 0x7;

        enum class Type : uint8_t
        {
            Red,
            Green,
            Blue,
            Alpha,
            Zero,
            One
        };

        constexpr ShaderComponentMapping(
            Type Source0 = Type::Red,
            Type Source1 = Type::Green,
            Type Source2 = Type ::Blue,
            Type Source3 = Type::Alpha) noexcept
        {
            m_Mapping = ((uint32_t(Source0) & c_ShiftMask)) |
                        ((uint32_t(Source1) & c_ShiftMask) << c_ShiftCount) |
                        ((uint32_t(Source2) & c_ShiftMask) << c_ShiftCount * 2) |
                        ((uint32_t(Source3) & c_ShiftMask) << c_ShiftCount * 3) |
                        ((1 & c_ShiftMask) << c_ShiftCount * 4);
        }

        constexpr Type Get(
            Type Component) noexcept
        {
            return Type((m_Mapping >> (uint32_t(Component) * c_ShiftCount)) & c_ShiftMask);
        }

        constexpr operator uint32_t() const noexcept
        {
            return m_Mapping;
        }

    private:
        uint32_t m_Mapping;
    };

    //

    struct CBVDesc
    {
        GpuResourceHandle Resource;
        size_t            Size = 0;
    };

    //

    struct SRVDesc
    {
        struct Buffer
        {
            size_t   FirstElement = 0;
            uint32_t Count        = 0;
            uint32_t SizeOfStruct = 0;
            bool     Raw : 1      = false;
        };

        struct Texture1D
        {
            uint32_t MostDetailedMip = 0;
            uint32_t MipLevels       = 1;
            float    MinLODClamp     = 0.f;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture2D
        {
            uint32_t MostDetailedMip = 0;
            uint32_t MipLevels       = 1;
            uint32_t PlaneSlice      = 0;
            float    MinLODClamp     = 0.f;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture2DMS
        {
        };

        struct Texture2DMSArray : Texture2DMS
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture3D
        {
            uint32_t MostDetailedMip = 0;
            uint32_t MipLevels       = 1;
            float    MinLODClamp     = 0.f;
        };

        struct TextureCube : Texture3D
        {
        };

        struct TextureCubeArray : TextureCube
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct RaytracingAccelerationStructure
        {
            GpuResourceHandle Resource;
        };

        using ViewVariant = std::variant<
            std::monostate,
            Buffer,
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture2DMS,
            Texture2DMSArray,
            Texture3D,
            TextureCube,
            TextureCubeArray,
            RaytracingAccelerationStructure>;

        //

        EResourceFormat        Format = EResourceFormat::Unknown;
        ShaderComponentMapping Mapping;
        ViewVariant            View;
    };

    //

    struct UAVDesc
    {
        struct Buffer
        {
            size_t   FirstElement  = 0;
            uint32_t Count         = 0;
            uint32_t SizeOfStruct  = 0;
            size_t   CounterOffset = 0;
            bool     Raw : 1       = false;
        };

        struct Texture1D
        {
            uint32_t MipSlice = 0;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture2D
        {
            uint32_t MipSlice   = 0;
            uint32_t PlaneSlice = 0;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture3D
        {
            uint32_t MipSlice    = 0;
            uint32_t FirstWSlice = 0;
            uint32_t Size        = 0;
        };

        using ViewVariant = std::variant<
            std::monostate,
            Buffer,
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture3D>;

        //

        EResourceFormat Format = EResourceFormat::Unknown;
        ViewVariant     View;
    };

    //

    enum class ERTClearType : uint8_t
    {
        /// <summary>
        /// Don't clear render target view
        /// </summary>
        Ignore,

        /// <summary>
        /// Clear render target view with the specified color if it was set, otherwise clear with the created clear color
        /// </summary>
        Color
    };

    struct RTVDesc
    {
        struct Buffer
        {
            size_t   FirstElement = 0;
            uint32_t Count        = 0;
        };

        struct Texture1D
        {
            uint32_t MipSlice = 0;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture2D
        {
            uint32_t MipSlice   = 0;
            uint32_t PlaneSlice = 0;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture2DMS
        {
        };

        struct Texture2DMSArray : Texture2DMS
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture3D
        {
            uint32_t MipSlice    = 0;
            uint32_t FirstWSlice = 0;
            uint32_t WSize       = 0;
        };

        using ViewVariant = std::variant<
            std::monostate,
            Buffer,
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture2DMS,
            Texture2DMSArray,
            Texture3D>;

        //

        ViewVariant View;

        // optional clear color, only used to hold information about the clear color for the user
        std::optional<Color4> ForceColor;
        ERTClearType          ClearType = ERTClearType::Ignore;

        EResourceFormat Format = EResourceFormat::Unknown;
    };

    //

    enum class EDSClearType : uint8_t
    {
        /// <summary>
        /// Don't clear the depth and stencil view
        /// </summary>
        Ignore,

        /// <summary>
        /// Clear the depth view only with the specified depth if it was set, otherwise clear with the created clear depth
        /// </summary>
        Depth,

        /// <summary>
        /// Clear the stencil view only with the specified stencil if it was set, otherwise clear with the created clear stencil
        /// </summary>
        Stencil,

        /// <summary>
        /// Clear both depth and stencil view with the specified depth and stencil if they were set, otherwise clear with the created clear depth and stencil
        /// </summary>
        DepthStencil,
    };

    struct DSVDesc
    {
        struct Texture1D
        {
            uint32_t MipSlice = 0;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture2D
        {
            uint32_t MipSlice = 0;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        struct Texture2DMS
        {
        };

        struct Texture2DMSArray : Texture2DMS
        {
            uint32_t StartSlice = 0;
            uint32_t Size       = 0;
        };

        using ViewVariant = std::variant<
            std::monostate,
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture2DMS,
            Texture2DMSArray>;

        //

        ViewVariant View;

        EDSClearType ClearType = EDSClearType::Ignore;

        // optional clear color, only used to hold information about the clear color for the user
        std::optional<float>   ForceDepth;
        std::optional<uint8_t> ForceStencil;

        EResourceFormat Format = EResourceFormat::Unknown;

        bool ReadOnlyDepth   : 1 = false;
        bool ReadOnlyStencil : 1 = false;
    };

    //

    using SRVDescOpt = std::optional<SRVDesc>;
    using UAVDescOpt = std::optional<UAVDesc>;
    using RTVDescOpt = std::optional<RTVDesc>;
    using DSVDescOpt = std::optional<DSVDesc>;

    using DescriptorViewDesc = std::variant<
        std::monostate,
        CBVDesc,
        SRVDescOpt,
        UAVDescOpt,
        RTVDescOpt,
        DSVDescOpt>;

    //

    struct SamplerDesc
    {
        ESamplerFilter       Filter         = ESamplerFilter::Comparison_MinMagMipLinear;
        ESamplerMode         AddressU       = ESamplerMode::Wrap;
        ESamplerMode         AddressV       = ESamplerMode::Wrap;
        ESamplerMode         AddressW       = ESamplerMode::Wrap;
        float                MipLODBias     = 0.f;
        uint32_t             MaxAnisotropy  = 16;
        ECompareFunc         ComparisonFunc = ECompareFunc::LessEqual;
        std::array<float, 4> BorderColor    = { 1.f, 1.f, 1.f, 1.f };
        float                MinLOD         = 0.f;
        float                MaxLOD         = FLT_MAX;

        constexpr operator bool() const noexcept
        {
            return Filter != ESamplerFilter::Unknown;
        }

        static const SamplerDesc Null;
    };
} // namespace Neon::RHI