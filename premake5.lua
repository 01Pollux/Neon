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
    

    group "Dependencies/ECS"
        include "Deps/Externals/flecs"
    
    group "Dependencies/UI"
        include "Deps/Externals/ImGui"

    group "Dependencies/Bullet3"
        include "Deps/Externals/Bullet3/src"

    group "Dependencies/ZipLib"
        include "Deps/Externals/ZipLib"
        
    group "Dependencies/Audio"
        include "Deps/Externals/libogg-1.3.0"
        include "Deps/Externals/vorbis"

        
    group "Neon"
        include "Neon/Engine"
        include "Neon/Core"
        include "Neon/Windowing"
    group ""

    group "Samples"
        include "Samples/HelloWorld"
    group ""
