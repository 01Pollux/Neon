#pragma once

#include <Math/Vector.hpp>

namespace Neon
{
    class Color4 final : public Vector4D
    {
    public:
        using Vector4D::Vector4D;

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);
        MVECTOR_IMPL_ACCESSOR(3, a);
    };

    class Color3 final : public Vector3D
    {
    public:
        using Vector3D::Vector3D;

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);
    };

    class Color4U8 final : public Neon::MVector<uint8_t, 4>
    {
    public:
        using MVector::MVector;

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);
        MVECTOR_IMPL_ACCESSOR(3, a);

        [[nodiscard]] static Color4U8 ToU8(Color4 Color)
        {
            Color *= 255;
            return Color4U8{
                static_cast<uint8_t>(Color.r()),
                static_cast<uint8_t>(Color.g()),
                static_cast<uint8_t>(Color.b()),
                static_cast<uint8_t>(Color.a())
            };
        }
        [[nodiscard]] static Color4 FromU8(const Color4U8& Color)
        {
            return Color4{
                Color.r() / 255.f,
                Color.g() / 255.f,
                Color.b() / 255.f,
                Color.a() / 255.f
            };
        }
    };

    class Color3U8 final : public Neon::MVector<uint8_t, 3>
    {
    public:
        using MVector::MVector;

        MVECTOR_IMPL_ACCESSOR(0, r);
        MVECTOR_IMPL_ACCESSOR(1, g);
        MVECTOR_IMPL_ACCESSOR(2, b);

        [[nodiscard]] static Color3U8 ToU8(Color4 Color)
        {
            Color *= 255;
            return Color4U8{
                static_cast<uint8_t>(Color.r()),
                static_cast<uint8_t>(Color.g()),
                static_cast<uint8_t>(Color.b())
            };
        }
        [[nodiscard]] static Color4 FromU8(const Color4U8& Color)
        {
            return Color4{
                Color.r() / 255.f,
                Color.g() / 255.f,
                Color.b() / 255.f
            };
        }
    };

    namespace Colors
    {
        static constexpr Color4 AliceBlue            = { 0.941176534f, 0.972549081f, 1.000000000f, 1.000000000f };
        static constexpr Color4 AntiqueWhite         = { 0.980392218f, 0.921568692f, 0.843137324f, 1.000000000f };
        static constexpr Color4 Aqua                 = { 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 Aquamarine           = { 0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f };
        static constexpr Color4 Azure                = { 0.941176534f, 1.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 Beige                = { 0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f };
        static constexpr Color4 Bisque               = { 1.000000000f, 0.894117713f, 0.768627524f, 1.000000000f };
        static constexpr Color4 Black                = { 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
        static constexpr Color4 BlanchedAlmond       = { 1.000000000f, 0.921568692f, 0.803921640f, 1.000000000f };
        static constexpr Color4 Blue                 = { 0.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 BlueViolet           = { 0.541176498f, 0.168627456f, 0.886274576f, 1.000000000f };
        static constexpr Color4 Brown                = { 0.647058845f, 0.164705887f, 0.164705887f, 1.000000000f };
        static constexpr Color4 BurlyWood            = { 0.870588303f, 0.721568644f, 0.529411793f, 1.000000000f };
        static constexpr Color4 CadetBlue            = { 0.372549027f, 0.619607866f, 0.627451003f, 1.000000000f };
        static constexpr Color4 Chartreuse           = { 0.498039246f, 1.000000000f, 0.000000000f, 1.000000000f };
        static constexpr Color4 Chocolate            = { 0.823529482f, 0.411764741f, 0.117647067f, 1.000000000f };
        static constexpr Color4 Coral                = { 1.000000000f, 0.498039246f, 0.313725501f, 1.000000000f };
        static constexpr Color4 CornflowerBlue       = { 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f };
        static constexpr Color4 Cornsilk             = { 1.000000000f, 0.972549081f, 0.862745166f, 1.000000000f };
        static constexpr Color4 Crimson              = { 0.862745166f, 0.078431375f, 0.235294133f, 1.000000000f };
        static constexpr Color4 Cyan                 = { 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 DarkBlue             = { 0.000000000f, 0.000000000f, 0.545098066f, 1.000000000f };
        static constexpr Color4 DarkCyan             = { 0.000000000f, 0.545098066f, 0.545098066f, 1.000000000f };
        static constexpr Color4 DarkGoldenrod        = { 0.721568644f, 0.525490224f, 0.043137256f, 1.000000000f };
        static constexpr Color4 DarkGray             = { 0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f };
        static constexpr Color4 DarkGreen            = { 0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f };
        static constexpr Color4 DarkKhaki            = { 0.741176486f, 0.717647076f, 0.419607878f, 1.000000000f };
        static constexpr Color4 DarkMagenta          = { 0.545098066f, 0.000000000f, 0.545098066f, 1.000000000f };
        static constexpr Color4 DarkOliveGreen       = { 0.333333343f, 0.419607878f, 0.184313729f, 1.000000000f };
        static constexpr Color4 DarkOrange           = { 1.000000000f, 0.549019635f, 0.000000000f, 1.000000000f };
        static constexpr Color4 DarkOrchid           = { 0.600000024f, 0.196078449f, 0.800000072f, 1.000000000f };
        static constexpr Color4 DarkRed              = { 0.545098066f, 0.000000000f, 0.000000000f, 1.000000000f };
        static constexpr Color4 DarkSalmon           = { 0.913725555f, 0.588235319f, 0.478431404f, 1.000000000f };
        static constexpr Color4 DarkSeaGreen         = { 0.560784340f, 0.737254918f, 0.545098066f, 1.000000000f };
        static constexpr Color4 DarkSlateBlue        = { 0.282352954f, 0.239215702f, 0.545098066f, 1.000000000f };
        static constexpr Color4 DarkSlateGray        = { 0.184313729f, 0.309803933f, 0.309803933f, 1.000000000f };
        static constexpr Color4 DarkTurquoise        = { 0.000000000f, 0.807843208f, 0.819607913f, 1.000000000f };
        static constexpr Color4 DarkViolet           = { 0.580392182f, 0.000000000f, 0.827451050f, 1.000000000f };
        static constexpr Color4 DeepPink             = { 1.000000000f, 0.078431375f, 0.576470613f, 1.000000000f };
        static constexpr Color4 DeepSkyBlue          = { 0.000000000f, 0.749019623f, 1.000000000f, 1.000000000f };
        static constexpr Color4 DimGray              = { 0.411764741f, 0.411764741f, 0.411764741f, 1.000000000f };
        static constexpr Color4 DodgerBlue           = { 0.117647067f, 0.564705908f, 1.000000000f, 1.000000000f };
        static constexpr Color4 Firebrick            = { 0.698039234f, 0.133333340f, 0.133333340f, 1.000000000f };
        static constexpr Color4 FloralWhite          = { 1.000000000f, 0.980392218f, 0.941176534f, 1.000000000f };
        static constexpr Color4 ForestGreen          = { 0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f };
        static constexpr Color4 Fuchsia              = { 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 Gainsboro            = { 0.862745166f, 0.862745166f, 0.862745166f, 1.000000000f };
        static constexpr Color4 GhostWhite           = { 0.972549081f, 0.972549081f, 1.000000000f, 1.000000000f };
        static constexpr Color4 Gold                 = { 1.000000000f, 0.843137324f, 0.000000000f, 1.000000000f };
        static constexpr Color4 Goldenrod            = { 0.854902029f, 0.647058845f, 0.125490203f, 1.000000000f };
        static constexpr Color4 Gray                 = { 0.501960814f, 0.501960814f, 0.501960814f, 1.000000000f };
        static constexpr Color4 Green                = { 0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f };
        static constexpr Color4 GreenYellow          = { 0.678431392f, 1.000000000f, 0.184313729f, 1.000000000f };
        static constexpr Color4 Honeydew             = { 0.941176534f, 1.000000000f, 0.941176534f, 1.000000000f };
        static constexpr Color4 HotPink              = { 1.000000000f, 0.411764741f, 0.705882370f, 1.000000000f };
        static constexpr Color4 IndianRed            = { 0.803921640f, 0.360784322f, 0.360784322f, 1.000000000f };
        static constexpr Color4 Indigo               = { 0.294117659f, 0.000000000f, 0.509803951f, 1.000000000f };
        static constexpr Color4 Ivory                = { 1.000000000f, 1.000000000f, 0.941176534f, 1.000000000f };
        static constexpr Color4 Khaki                = { 0.941176534f, 0.901960850f, 0.549019635f, 1.000000000f };
        static constexpr Color4 Lavender             = { 0.901960850f, 0.901960850f, 0.980392218f, 1.000000000f };
        static constexpr Color4 LavenderBlush        = { 1.000000000f, 0.941176534f, 0.960784376f, 1.000000000f };
        static constexpr Color4 LawnGreen            = { 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f };
        static constexpr Color4 LemonChiffon         = { 1.000000000f, 0.980392218f, 0.803921640f, 1.000000000f };
        static constexpr Color4 LightBlue            = { 0.678431392f, 0.847058892f, 0.901960850f, 1.000000000f };
        static constexpr Color4 LightCoral           = { 0.941176534f, 0.501960814f, 0.501960814f, 1.000000000f };
        static constexpr Color4 LightCyan            = { 0.878431439f, 1.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 LightGoldenrodYellow = { 0.980392218f, 0.980392218f, 0.823529482f, 1.000000000f };
        static constexpr Color4 LightGreen           = { 0.564705908f, 0.933333397f, 0.564705908f, 1.000000000f };
        static constexpr Color4 LightGray            = { 0.827451050f, 0.827451050f, 0.827451050f, 1.000000000f };
        static constexpr Color4 LightPink            = { 1.000000000f, 0.713725507f, 0.756862819f, 1.000000000f };
        static constexpr Color4 LightSalmon          = { 1.000000000f, 0.627451003f, 0.478431404f, 1.000000000f };
        static constexpr Color4 LightSeaGreen        = { 0.125490203f, 0.698039234f, 0.666666687f, 1.000000000f };
        static constexpr Color4 LightSkyBlue         = { 0.529411793f, 0.807843208f, 0.980392218f, 1.000000000f };
        static constexpr Color4 LightSlateGray       = { 0.466666698f, 0.533333361f, 0.600000024f, 1.000000000f };
        static constexpr Color4 LightSteelBlue       = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
        static constexpr Color4 LightYellow          = { 1.000000000f, 1.000000000f, 0.878431439f, 1.000000000f };
        static constexpr Color4 Lime                 = { 0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };
        static constexpr Color4 LimeGreen            = { 0.196078449f, 0.803921640f, 0.196078449f, 1.000000000f };
        static constexpr Color4 Linen                = { 0.980392218f, 0.941176534f, 0.901960850f, 1.000000000f };
        static constexpr Color4 Magenta              = { 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 Maroon               = { 0.501960814f, 0.000000000f, 0.000000000f, 1.000000000f };
        static constexpr Color4 MediumAquamarine     = { 0.400000036f, 0.803921640f, 0.666666687f, 1.000000000f };
        static constexpr Color4 MediumBlue           = { 0.000000000f, 0.000000000f, 0.803921640f, 1.000000000f };
        static constexpr Color4 MediumOrchid         = { 0.729411781f, 0.333333343f, 0.827451050f, 1.000000000f };
        static constexpr Color4 MediumPurple         = { 0.576470613f, 0.439215720f, 0.858823597f, 1.000000000f };
        static constexpr Color4 MediumSeaGreen       = { 0.235294133f, 0.701960802f, 0.443137288f, 1.000000000f };
        static constexpr Color4 MediumSlateBlue      = { 0.482352972f, 0.407843173f, 0.933333397f, 1.000000000f };
        static constexpr Color4 MediumSpringGreen    = { 0.000000000f, 0.980392218f, 0.603921592f, 1.000000000f };
        static constexpr Color4 MediumTurquoise      = { 0.282352954f, 0.819607913f, 0.800000072f, 1.000000000f };
        static constexpr Color4 MediumVioletRed      = { 0.780392230f, 0.082352944f, 0.521568656f, 1.000000000f };
        static constexpr Color4 MidnightBlue         = { 0.098039225f, 0.098039225f, 0.439215720f, 1.000000000f };
        static constexpr Color4 MintCream            = { 0.960784376f, 1.000000000f, 0.980392218f, 1.000000000f };
        static constexpr Color4 MistyRose            = { 1.000000000f, 0.894117713f, 0.882353008f, 1.000000000f };
        static constexpr Color4 Moccasin             = { 1.000000000f, 0.894117713f, 0.709803939f, 1.000000000f };
        static constexpr Color4 NavajoWhite          = { 1.000000000f, 0.870588303f, 0.678431392f, 1.000000000f };
        static constexpr Color4 Navy                 = { 0.000000000f, 0.000000000f, 0.501960814f, 1.000000000f };
        static constexpr Color4 None                 = { 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f };
        static constexpr Color4 OldLace              = { 0.992156923f, 0.960784376f, 0.901960850f, 1.000000000f };
        static constexpr Color4 Olive                = { 0.501960814f, 0.501960814f, 0.000000000f, 1.000000000f };
        static constexpr Color4 OliveDrab            = { 0.419607878f, 0.556862772f, 0.137254909f, 1.000000000f };
        static constexpr Color4 Orange               = { 1.000000000f, 0.647058845f, 0.000000000f, 1.000000000f };
        static constexpr Color4 OrangeRed            = { 1.000000000f, 0.270588249f, 0.000000000f, 1.000000000f };
        static constexpr Color4 Orchid               = { 0.854902029f, 0.439215720f, 0.839215755f, 1.000000000f };
        static constexpr Color4 PaleGoldenrod        = { 0.933333397f, 0.909803987f, 0.666666687f, 1.000000000f };
        static constexpr Color4 PaleGreen            = { 0.596078455f, 0.984313786f, 0.596078455f, 1.000000000f };
        static constexpr Color4 PaleTurquoise        = { 0.686274529f, 0.933333397f, 0.933333397f, 1.000000000f };
        static constexpr Color4 PaleVioletRed        = { 0.858823597f, 0.439215720f, 0.576470613f, 1.000000000f };
        static constexpr Color4 PapayaWhip           = { 1.000000000f, 0.937254965f, 0.835294187f, 1.000000000f };
        static constexpr Color4 PeachPuff            = { 1.000000000f, 0.854902029f, 0.725490212f, 1.000000000f };
        static constexpr Color4 Peru                 = { 0.803921640f, 0.521568656f, 0.247058839f, 1.000000000f };
        static constexpr Color4 Pink                 = { 1.000000000f, 0.752941251f, 0.796078503f, 1.000000000f };
        static constexpr Color4 Plum                 = { 0.866666734f, 0.627451003f, 0.866666734f, 1.000000000f };
        static constexpr Color4 PowderBlue           = { 0.690196097f, 0.878431439f, 0.901960850f, 1.000000000f };
        static constexpr Color4 Purple               = { 0.501960814f, 0.000000000f, 0.501960814f, 1.000000000f };
        static constexpr Color4 Red                  = { 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
        static constexpr Color4 RosyBrown            = { 0.737254918f, 0.560784340f, 0.560784340f, 1.000000000f };
        static constexpr Color4 RoyalBlue            = { 0.254901975f, 0.411764741f, 0.882353008f, 1.000000000f };
        static constexpr Color4 SaddleBrown          = { 0.545098066f, 0.270588249f, 0.074509807f, 1.000000000f };
        static constexpr Color4 Salmon               = { 0.980392218f, 0.501960814f, 0.447058856f, 1.000000000f };
        static constexpr Color4 SandyBrown           = { 0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f };
        static constexpr Color4 SeaGreen             = { 0.180392161f, 0.545098066f, 0.341176480f, 1.000000000f };
        static constexpr Color4 SeaShell             = { 1.000000000f, 0.960784376f, 0.933333397f, 1.000000000f };
        static constexpr Color4 Sienna               = { 0.627451003f, 0.321568638f, 0.176470593f, 1.000000000f };
        static constexpr Color4 Silver               = { 0.752941251f, 0.752941251f, 0.752941251f, 1.000000000f };
        static constexpr Color4 SkyBlue              = { 0.529411793f, 0.807843208f, 0.921568692f, 1.000000000f };
        static constexpr Color4 SlateBlue            = { 0.415686309f, 0.352941185f, 0.803921640f, 1.000000000f };
        static constexpr Color4 SlateGray            = { 0.439215720f, 0.501960814f, 0.564705908f, 1.000000000f };
        static constexpr Color4 Snow                 = { 1.000000000f, 0.980392218f, 0.980392218f, 1.000000000f };
        static constexpr Color4 SpringGreen          = { 0.000000000f, 1.000000000f, 0.498039246f, 1.000000000f };
        static constexpr Color4 SteelBlue            = { 0.274509817f, 0.509803951f, 0.705882370f, 1.000000000f };
        static constexpr Color4 Tan                  = { 0.823529482f, 0.705882370f, 0.549019635f, 1.000000000f };
        static constexpr Color4 Teal                 = { 0.000000000f, 0.501960814f, 0.501960814f, 1.000000000f };
        static constexpr Color4 Thistle              = { 0.847058892f, 0.749019623f, 0.847058892f, 1.000000000f };
        static constexpr Color4 Tomato               = { 1.000000000f, 0.388235331f, 0.278431386f, 1.000000000f };
        static constexpr Color4 Transparent          = { 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f };
        static constexpr Color4 Turquoise            = { 0.250980407f, 0.878431439f, 0.815686345f, 1.000000000f };
        static constexpr Color4 Violet               = { 0.933333397f, 0.509803951f, 0.933333397f, 1.000000000f };
        static constexpr Color4 Wheat                = { 0.960784376f, 0.870588303f, 0.701960802f, 1.000000000f };
        static constexpr Color4 White                = { 1.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
        static constexpr Color4 WhiteSmoke           = { 0.960784376f, 0.960784376f, 0.960784376f, 1.000000000f };
        static constexpr Color4 Yellow               = { 1.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };
        static constexpr Color4 YellowGreen          = { 0.603921592f, 0.803921640f, 0.196078449f, 1.000000000f };
    } // namespace Colors
} // namespace Neon