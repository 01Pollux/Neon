project "LibOgg"
	kind "StaticLib"
	language "C"
	compileas "C"
	staticruntime "On"
	
	systemversion "latest"
	
	common_dir_setup()
	
	files
	{
		"**.cpp",
		"**.c",
		"**.h",
		"**.hpp",
	}
	
    includedirs
    {
        "%{CommonDir.Deps.Inc}/LibOgg"
    }

	disablewarnings
	{
		"4244",
		"4267",
		"4305",
		"4554",
		"4996",
	}