project "NeonGraphics"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("GraphicsPCH")
    common_neon()
    
    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}"
    }

    links
    {
        "%{CommonDir.Deps.Libs}/ShaderConductor/ShaderConductor.lib"
    }