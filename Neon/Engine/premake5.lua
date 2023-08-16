project "NeonEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("EnginePCH")
    common_neon()

    removefiles
    {
        "**/Editor/**"
    }

   
project "NeonEditor"
    select_launch_kind()
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
    defines "NEON_EDITOR"
    copy_engine_resources("Editor")
    link_engine_library_no_engine()

	common_dir_setup()
    common_add_pch("EnginePCH")
    common_neon()
