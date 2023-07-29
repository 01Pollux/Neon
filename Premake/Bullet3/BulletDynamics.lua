project "BulletDynamics"
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
		"Dynamics/*.cpp",
        "Dynamics/*.h",
        "ConstraintSolver/*.cpp",
        "ConstraintSolver/*.h",
        "Featherstone/*.cpp",
        "Featherstone/*.h",
        "MLCPSolvers/*.cpp",
        "MLCPSolvers/*.h",
        "Vehicle/*.cpp",
        "Vehicle/*.h",
        "Character/*.cpp",
        "Character/*.h"
	}
