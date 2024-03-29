project "NeonWindowing"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("WindowPCH")
    common_neon()
    
    filter { "system:windows" }
        links
        {
            "Comctl32"
        }
    filter {}

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Coroutines}",
        "%{CommonDir.Neon.Windowing}"
    }