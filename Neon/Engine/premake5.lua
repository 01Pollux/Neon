project "NeonEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"

	common_dir_setup()
    common_add_pch("EnginePCH")
    common_neon()

    files
    {
		"**.cpp",
		"**.hpp",
		"**.natvis"
	}

    removefiles
    {
        "**/Editor/**.hpp",
        "**/Editor/**.cpp",
    }

   
project "NeonEditor"
    select_launch_kind()
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
    defines "NEON_EDITOR"
    copy_engine_resources_to("Editor", "Editor")
    copy_directory_to_target_dir("%{wks.location}Contents/EditorTemplates/*.ntarch", "Templates")
    link_engine_library_no_engine()

	common_dir_setup()
    common_add_pch("EnginePCH")
    common_neon()
    
    links
    {
        "NeonWindowing"
    }