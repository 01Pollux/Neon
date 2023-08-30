include "neon-config.lua"
include "neon-functions.lua"

workspace "Neon"
    architecture "x64"
    startproject "NeonEditor"

    configurations
    {
        "Debug", -- Edit and continue
        "Release", -- Optimized + Asan
        "ReleaseOpt", -- Optimized + symbols
        "Dist" -- Optimized + No symbols
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
        sanitize
        {
            "Address"
        }
    filter {}
    
    filter { "configurations:ReleaseOpt" }
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
    

    group "Dependencies/ECS"
        include "Deps/Externals/flecs"
    
    group "Dependencies/Logging"
        include "Deps/Externals/spdlog"
    
    group "Dependencies/UI"
        include "Deps/Externals/ImGui"
        include "Deps/Externals/glfw"

    group "Dependencies/Bullet3"
        include "Deps/Externals/Bullet3/src"

    group "Dependencies/Audio"
        include "Deps/Externals/libogg-1.3.0"
        include "Deps/Externals/vorbis"

        
    group "Neon"
        include "Neon/ScriptCore"
        include "Neon/Core"
        include "Neon/Coroutines"
        include "Neon/Resource"
        include "Neon/Windowing"
        include "Neon/Graphics"
        include "Neon/Engine"
    group ""

    group "Neon/Tools"
--        include "Neon/Tools/pakc"
    group ""

    group "Samples"
--        include "Samples/BasicAsset"
--        include "Samples/HelloWorld"
--        include "Samples/FlappyBirdClone"
        include "Samples/CSharpTemplate"
    group ""
