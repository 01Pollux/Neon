project "NeonEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("EnginePCH")
    common_neon()

    links
    {
        "NeonCore",
        "NeonCoroutines",
        "NeonResource",
        "NeonWindowing",
        "NeonGraphics"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Coroutines}",
        "%{CommonDir.Neon.Resource}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}",
        "%{CommonDir.Neon.Engine}"
    }