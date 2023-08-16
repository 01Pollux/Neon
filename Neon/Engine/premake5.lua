function neon_engine_project_setup()
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("EnginePCH")
    common_neon()

    links
    {
        "Bullet3Common",
        "Bullet3Collision",
        "Bullet3Dynamics",
        "Bullet3Geometry",
        "Bullet3Serialize",
        "BulletCollision",
        "BulletDynamics",
        "BulletInverseDynamics",
        "BulletSoftBody",
        "BulletLinearMath",

        "ImGui",

        "NeonCore",
        "NeonCoroutines",
        "NeonResource",
        "NeonWindowing",
        "NeonGraphics"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Coroutines}",
        "%{CommonDir.Neon.Resource}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}",
        "%{CommonDir.Neon.Engine}",

        "%{CommonDir.Deps.Inc}/Bullet3"
    }
end

project "NeonEngine"
    kind "StaticLib"
    neon_engine_project_setup()
    removefiles
    {
        "**/Editor/**"
    }
   
project "NeonEditor"
    neon_engine_project_setup()
    defines "NEON_EDITOR"
    setup_runtime_engine("Editor")
