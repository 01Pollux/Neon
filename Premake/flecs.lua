project "Flecs"
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	staticruntime "On"
    architecture "x86_64"
	
	common_dir_setup()

	files
	{
		"include/**.h",
		"include/**.hpp",
		"include/**.cpp",
		"include/**.c",

		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
		"src/**.c",

		"**.natvis"
	}

    includedirs
    {
        "%{prj.location}/include",
        "%{prj.location}"
    }

    defines "flecs_STATIC"
