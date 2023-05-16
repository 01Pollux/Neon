project "NeonResource"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("ResourcePCH")
    common_neon()

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Resource}"
    }