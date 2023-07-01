project "NeonGraphics"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("GraphicsPCH")
    common_neon()
    
    links
    {
        "NeonCore",
        "NeonCoroutines",
        "NeonWindowing"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Coroutines}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}"
    }

    links
    {
        "%{CommonDir.Deps.Libs}/DxC/lib/x64/dxcompiler.lib"
    }