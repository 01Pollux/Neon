include "neon-config.lua"
include "neon-functions.lua"

workspace "Neon"
    architecture "x86_64"
    startproject "HelloWorld"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }
    
    defines "NOMINMAX"
    
	filter "system:windows"
		systemversion "latest"
		defines "_WIN32"
		defines "WIN32"
    filter {}
            
    filter "configurations:Debug"
        defines "NEON_DEBUG"
        runtime "Debug"
        symbols "On"
    filter {}

    filter "configurations:Release"
        defines "NEON_RELEASE"
        runtime "Release"
        optimize "On"
    filter {}

    filter "configurations:Dist"
        defines "NEON_DIST"
        runtime "Release"
        optimize "On"
        symbols "Off"
    filter {}

    include "Neon-Module"

    group "Samples"
        include "Samples/HelloWorld"
    group ""
