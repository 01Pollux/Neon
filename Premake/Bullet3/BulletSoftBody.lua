project "BulletSoftBody"
	kind "StaticLib"
	language "C++"
	staticruntime "On"

	systemversion "latest"
	cppdialect "C++17"
	
	common_dir_setup()

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
        "%{IncludeDir.NeonDeps}/Externals/Bullet3/src"
	}

	files
	{
		"**.cpp",
        "BulletReducedDeformableBody/**.cpp",
		"**.h"
	}
