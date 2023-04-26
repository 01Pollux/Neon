project "NeonGraphics"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x86_64"
    
	common_dir_setup()
    common_add_pch("GraphicsPCH")
    common_neon()
    
    links
    {
        "d3d12",
        "dxgi"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}"
    }