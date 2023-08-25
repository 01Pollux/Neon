project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	staticruntime "On"
    architecture "x64"
	
	common_dir_setup()

	files
	{
		"*.h",
		"*.cpp",
		"**.natvis",

		"misc/cpp/imgui_stdlib.hpp",
		"misc/cpp/imgui_stdlib.cpp",
		
		"backends/imgui_impl_glfw.h",
		"backends/imgui_impl_glfw.cpp"
	}

	filter "system:windows"
		files
		{
			"backends/imgui_impl_dx12.h",
			"backends/imgui_impl_dx12.cpp"
		}
	filter {}

    includedirs
    {
        "%{prj.location}",
		"%{CommonDir.Deps.Inc}"
    }