#include <PakCPCH.hpp>
#include <Parser/RHI.hpp>
#include <map>

#include <Log/Logger.hpp>

namespace PakC
{
    using namespace Neon;

    static std::map<StringU8, RHI::ESamplerFilter> s_SamplerMap{
        { "MinMagMipPoint", RHI::ESamplerFilter::MinMagMipPoint },
        { "MinMagPoint_MipLinear", RHI::ESamplerFilter::MinMagPoint_MipLinear },
        { "MinPoint_MagLinear_MipPoint", RHI::ESamplerFilter::MinPoint_MagLinear_MipPoint },
        { "MinPoint_MagMipLinear", RHI::ESamplerFilter::MinPoint_MagMipLinear },
        { "MinLinear_MagMipPoint", RHI::ESamplerFilter::MinLinear_MagMipPoint },
        { "MinLinear_MagPoint_MipLinear", RHI::ESamplerFilter::MinLinear_MagPoint_MipLinear },
        { "MinMagLinear_MipPoint", RHI::ESamplerFilter::MinMagLinear_MipPoint },
        { "MinMagMipLinear", RHI::ESamplerFilter::MinMagMipLinear },
        { "Anisotropic", RHI::ESamplerFilter::Anisotropic },
        { "Comparison_MinMagMipPoint", RHI::ESamplerFilter::Comparison_MinMagMipPoint },
        { "Comparison_MinMagPoint_MipLinear", RHI::ESamplerFilter::Comparison_MinMagPoint_MipLinear },
        { "Comparison_MinPoint_MagLinear_MipPoint", RHI::ESamplerFilter::Comparison_MinPoint_MagLinear_MipPoint },
        { "Comparison_MinPoint_MagMipLinear", RHI::ESamplerFilter::Comparison_MinPoint_MagMipLinear },
        { "Comparison_MinLinear_MagMipPoint", RHI::ESamplerFilter::Comparison_MinLinear_MagMipPoint },
        { "Comparison_MinLinear_MagPoint_MipLinear", RHI::ESamplerFilter::Comparison_MinLinear_MagPoint_MipLinear },
        { "Comparison_MinMagLinear_MipPoint", RHI::ESamplerFilter::Comparison_MinMagLinear_MipPoint },
        { "Comparison_MinMagMipLinear", RHI::ESamplerFilter::Comparison_MinMagMipLinear },
        { "Comparison_Anisotropic", RHI::ESamplerFilter::Comparison_Anisotropic },
        { "Minimum_MinMagMipPoint", RHI::ESamplerFilter::Minimum_MinMagMipPoint },
        { "Minimum_MinMagPoint_MipLinear", RHI::ESamplerFilter::Minimum_MinMagPoint_MipLinear },
        { "Minimum_MinPoint_MagLinear_MipPoint", RHI::ESamplerFilter::Minimum_MinPoint_MagLinear_MipPoint },
        { "Minimum_MinPoint_MagMipLinear", RHI::ESamplerFilter::Minimum_MinPoint_MagMipLinear },
        { "Minimum_MinLinear_MagMipPoint", RHI::ESamplerFilter::Minimum_MinLinear_MagMipPoint },
        { "Minimum_MinLinear_MagPoint_MipLinear", RHI::ESamplerFilter::Minimum_MinLinear_MagPoint_MipLinear },
        { "Minimum_MinMagLinear_MipPoint", RHI::ESamplerFilter::Minimum_MinMagLinear_MipPoint },
        { "Minimum_MinMagMipLinear", RHI::ESamplerFilter::Minimum_MinMagMipLinear },
        { "Minimum_Anisotropic", RHI::ESamplerFilter::Minimum_Anisotropic },
        { "Maximum_MinMagMipPoint", RHI::ESamplerFilter::Maximum_MinMagMipPoint },
        { "Maximum_MinMagPoint_MipLinear", RHI::ESamplerFilter::Maximum_MinMagPoint_MipLinear },
        { "Maximum_MinPoint_MagLinear_MipPoint", RHI::ESamplerFilter::Maximum_MinPoint_MagLinear_MipPoint },
        { "Maximum_MinPoint_MagMipLinear", RHI::ESamplerFilter::Maximum_MinPoint_MagMipLinear },
        { "Maximum_MinLinear_MagMipPoint", RHI::ESamplerFilter::Maximum_MinLinear_MagMipPoint },
        { "Maximum_MinLinear_MagPoint_MipLinear", RHI::ESamplerFilter::Maximum_MinLinear_MagPoint_MipLinear },
        { "Maximum_MinMagLinear_MipPoint", RHI::ESamplerFilter::Maximum_MinMagLinear_MipPoint },
        { "Maximum_MinMagMipLinear", RHI::ESamplerFilter::Maximum_MinMagMipLinear },
        { "Maximum_Anisotropic", RHI::ESamplerFilter::Maximum_Anisotropic }
    };

    //

    static std::map<StringU8, RHI::ESamplerMode> s_SamplerAddressModeMap{
        { "Wrap", RHI::ESamplerMode::Wrap },
        { "Mirror", RHI::ESamplerMode::Mirror },
        { "Clamp", RHI::ESamplerMode::Clamp },
        { "Border", RHI::ESamplerMode::Border },
        { "MirrorOnce", RHI::ESamplerMode::MirrorOnce }
    };

    //

    static std::map<StringU8, RHI::ECompareFunc> s_CompareFuncMap{
        { "Never", RHI::ECompareFunc::Never },
        { "Less", RHI::ECompareFunc::Less },
        { "Equal", RHI::ECompareFunc::Equal },
        { "LessEqual", RHI::ECompareFunc::LessEqual },
        { "Greater", RHI::ECompareFunc::Greater },
        { "NotEqual", RHI::ECompareFunc::NotEqual },
        { "GreaterEqual", RHI::ECompareFunc::GreaterEqual },
        { "Always", RHI::ECompareFunc::Always }
    };

    //

    RHI::ESamplerFilter ConvertSamplerFilter(
        const Neon::StringU8& Filter)
    {
        auto Iter = s_SamplerMap.find(Filter);
        NEON_VALIDATE(Iter != s_SamplerMap.end(), "Filter '{}' not found", Filter);
        return Iter->second;
    }

    RHI::ESamplerMode ConvertSamplerAddressMode(
        const Neon::StringU8& AddressMode)
    {
        auto Iter = s_SamplerAddressModeMap.find(AddressMode);
        NEON_VALIDATE(Iter != s_SamplerAddressModeMap.end(), "AddressMode '{}' not found", AddressMode);
        return Iter->second;
    }

    RHI::ECompareFunc ConvertCompareFunc(
        const Neon::StringU8& CompareFunction)
    {
        auto Iter = s_CompareFuncMap.find(CompareFunction);
        NEON_VALIDATE(Iter != s_CompareFuncMap.end(), "CompareFunction '{}' not found", CompareFunction);
        return Iter->second;
    }
} // namespace PakC