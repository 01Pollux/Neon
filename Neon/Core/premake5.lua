project "Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x86_64"
    
	common_dir_setup()
    common_add_pch("CorePCH")
    
    files
    {
        "**.cpp",
        "**.hpp",
        "**.natvis"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Deps.Inc}",
        "%{prj.location}"
    }