project "lzma"
	kind "StaticLib"
	language "C"
	
	systemversion "latest"
	staticruntime "On"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	files
	{
		"*.c",
		"*.h"
	}

	defines "_CRT_SECURE_NO_WARNINGS"

    disablewarnings
    {
		"4244",
        "4267",
        "4996"
    }
