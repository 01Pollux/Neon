include "neon-config.lua"
include "neon-functions.lua"

workspace "Neon"
    architecture "x64"
    startproject "NeonEditor"

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
        defines "NEON_PLATFORM_WINDOWS"
    filter {}
            
    filter "configurations:Debug"
        defines "NEON_DEBUG"
        runtime "Debug"
        symbols "On"
        editAndContinue "On"
        sanitize
        {
            "Address"
        }
    filter {}

    filter "configurations:Release"
        defines "NEON_RELEASE"
        runtime "Debug"
        symbols "On"
        optimize "On"
        editAndContinue "Off"
        sanitize
        {
            "Address"
        }
    filter {}

    filter "configurations:Dist"
        defines "NEON_DIST"
        runtime "Release"
        optimize "On"
        symbols "Off"
        editAndContinue "Off"
    filter {}
    

    group "Dependencies/ECS"
        include "Deps/Externals/flecs"
    
    group "Dependencies/Logging"
        include "Deps/Externals/spdlog"
    
    group "Dependencies/UI"
        include "Deps/Externals/ImGui"

    group "Dependencies/Bullet3"
        include "Deps/Externals/Bullet3/src"

    group "Dependencies/Audio"
        include "Deps/Externals/libogg-1.3.0"
        include "Deps/Externals/vorbis"

        
    group "Neon"
        include "Neon/Core"
        include "Neon/Coroutines"
        include "Neon/Resource"
        include "Neon/Windowing"
        include "Neon/Graphics"
        include "Neon/Engine"
        include "Neon/Editor"
    group ""

    group "Neon/Tools"
--        include "Neon/Tools/pakc"
    group ""

    group "Samples"
--        include "Samples/BasicAsset"
--        include "Samples/HelloWorld"
        include "Samples/FlappyBirdClone"
    group ""
