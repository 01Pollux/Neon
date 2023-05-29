#pragma once

#include <RHI/Resource/Common.hpp>
#include <variant>

namespace Neon::RHI
{
    struct ShaderComponentMapping
    {
    public:
        static constexpr uint32_t c_ShfitCount = 3;
        static constexpr uint32_t c_ShfitMask  = 0x7;

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
            m_Mapping = ((uint32_t(Source0) & c_ShfitMask)) |
                        ((uint32_t(Source1) & c_ShfitMask) << c_ShfitCount) |
                        ((uint32_t(Source2) & c_ShfitMask) << c_ShfitCount * 2) |
                        ((uint32_t(Source3) & c_ShfitMask) << c_ShfitCount * 3);
        }

        constexpr Type Get(
            Type Component) noexcept
        {
            return Type((m_Mapping >> (uint32_t(Component) * c_ShfitCount)) & c_ShfitMask);
        }

        constexpr operator uint32_t() const noexcept
        {
            return m_Mapping;
        }

    private:
        uint32_t m_Mapping;
    };

    //

    struct UAVDesc
    {
        struct Buffer
        {
            size_t   FirstElement;
            uint32_t Count;
            uint32_t SizeOfStruct;
            size_t   CounterOffset;
            bool     Raw : 1;
        };

        struct Texture1D
        {
            uint32_t MipSlice;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture2D
        {
            uint32_t MipSlice;
            uint32_t PlaneSlice;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture3D
        {
            uint32_t MipSlice;
            uint32_t FirstWSlice;
            uint32_t Size;
        };

        using ViewVariant = std::variant<
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

    struct SRVDesc
    {
        struct Buffer
        {
            size_t   FirstElement;
            uint32_t Count;
            uint32_t SizeOfStruct;
            bool     Raw : 1;
        };

        struct Texture1D
        {
            uint32_t MostDetailedMip;
            uint32_t MipLevels;
            float    MinLODClamp;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture2D
        {
            uint32_t MostDetailedMip;
            uint32_t MipLevels;
            uint32_t PlaneSlice;
            float    MinLODClamp;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture2DMS
        {
        };

        struct Texture2DMSArray : Texture2DMS
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture3D
        {
            uint32_t MostDetailedMip;
            uint32_t MipLevels;
            float    MinLODClamp;
        };

        struct TextureCube : Texture3D
        {
        };

        struct TextureCubeArray : TextureCube
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct RaytracingAccelerationStructure
        {
            GpuResourceHandle Resource;
        };

        using ViewVariant = std::variant<
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

    struct CBVDesc
    {
        GpuResourceHandle Resource;
        size_t            Size;
    };

    //

    struct RTVDesc
    {
        struct Buffer
        {
            size_t   FirstElement;
            uint32_t Count;
        };

        struct Texture1D
        {
            uint32_t MipSlice;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture2D
        {
            uint32_t MipSlice;
            uint32_t PlaneSlice;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture2DMS
        {
        };

        struct Texture2DMSArray : Texture2DMS
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture3D
        {
            uint32_t MipSlice;
            uint32_t FirstWSlice;
            uint32_t WSize;
        };

        using ViewVariant = std::variant<
            Buffer,
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture2DMS,
            Texture2DMSArray,
            Texture3D>;

        //

        EResourceFormat Format = EResourceFormat::Unknown;
        ViewVariant     View;
    };

    //

    struct DSVDesc
    {
        struct Texture1D
        {
            uint32_t MipSlice;
        };

        struct Texture1DArray : Texture1D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture2D
        {
            uint32_t MipSlice;
        };

        struct Texture2DArray : Texture2D
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        struct Texture2DMS
        {
        };

        struct Texture2DMSArray : Texture2DMS
        {
            uint32_t StartSlice = 0;
            uint32_t Size;
        };

        using ViewVariant = std::variant<
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture2DMS,
            Texture2DMSArray>;

        //

        EResourceFormat Format = EResourceFormat::Unknown;
        ViewVariant     View;

        bool OnlyDepth   : 1;
        bool OnlyStencil : 1;
    };

    //

    using DescriptorViewDesc = std::variant<
        CBVDesc,
        SRVDesc,
        UAVDesc,
        RTVDesc,
        DSVDesc>;

    //

    struct SamplerDesc
    {
        ESamplerFilter Filter         = ESamplerFilter::Comparison_MinMagMipLinear;
        ESamplerMode   AddressU       = ESamplerMode::Wrap;
        ESamplerMode   AddressV       = ESamplerMode::Wrap;
        ESamplerMode   AddressW       = ESamplerMode::Wrap;
        float          MipLODBias     = 0.f;
        uint32_t       MaxAnisotropy  = 1;
        ESamplerCmp    ComparisonFunc = ESamplerCmp::Less;
        float          BorderColor[4] = { 1.f, 0.f, 1.f, 1.f };
        float          MinLOD         = -16.f;
        float          MaxLOD         = 15.99f;
    };
} // namespace Neon::RHI