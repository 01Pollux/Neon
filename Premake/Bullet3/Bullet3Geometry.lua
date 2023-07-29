project "Bullet3Geometry"
	kind "StaticLib"
	language "C++"
	staticruntime "On"

	systemversion "latest"
	cppdialect "C++17"
	
	common_dir_setup()
    defines "BT_USE_DOUBLE_PRECISION"

    disablewarnings
    {
		"4056",
		"4018",
		"4244",
		"4267",
		"4267",
		"4305"
	}

	includedirs
	{
        "%{CommonDir.Deps.Inc}/Bullet3"
	}

	files
	{
		"**.cpp",
		"**.h"
	}
