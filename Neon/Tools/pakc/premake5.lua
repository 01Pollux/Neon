project "pakc"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("PakCPCH")
    common_neon()
    
    link_engine_library()
    copy_engine_resources()