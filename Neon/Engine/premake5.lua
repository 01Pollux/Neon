project "NeonEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x86_64"
    
	common_dir_setup()
    common_add_pch("EnginePCH")
    common_neon()

    links
    {
        "NeonCore",
        "NeonResource",
        "NeonWindowing",
        "NeonGraphics",
        "NeonRendering"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Resource}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}",
        "%{CommonDir.Neon.Rendering}",
        "%{CommonDir.Neon.Engine}"
    }