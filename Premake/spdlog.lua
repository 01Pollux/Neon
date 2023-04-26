project "spdlog"
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	staticruntime "On"
    architecture "x86_64"

	common_dir_setup()

	files
	{
		"include/**.h",
		"src/**.cpp",
		"**.natvis"
	}

    includedirs
    {
        "%{prj.location}/include",
        "%{prj.location}"
    }

    defines "SPDLOG_COMPILED_LIB"
