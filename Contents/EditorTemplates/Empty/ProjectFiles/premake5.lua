OutputDir = "%{cfg.buildcfg}"

include "neon-functions.lua"

workspace "%PROJECT_NAME%"
    architecture "x64"

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

	filter { "system:windows" }
		systemversion "latest"
		defines "_WIN32"
		defines "WIN32"
        defines "NEON_PLATFORM_WINDOWS"
    filter {}

            
    filter { "configurations:Debug" }
        defines "NEON_DEBUG"
        runtime "Debug"
        symbols "On"
        editAndContinue "On"
    filter {}
    
    filter { "configurations:Release" }
        defines "NEON_RELEASE"
        runtime "Release"
        symbols "On"
        optimize "On"
    filter {}

    filter { "configurations:Dist" }
        defines "NEON_DIST"
        runtime "Release"
        optimize "On"
        symbols "Off"
    filter {}
    
    include "Neon-CSharp"
