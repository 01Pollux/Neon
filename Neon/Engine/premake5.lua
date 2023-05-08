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
        "NeonWindowing",
        "NeonGraphics",
        "NeonResource"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}",
        "%{CommonDir.Neon.Resource}",
        "%{CommonDir.Neon.Engine}"
    }