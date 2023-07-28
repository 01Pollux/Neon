project "NeonResource"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"
    
	common_dir_setup()
    common_add_pch("ResourcePCH")
    common_neon()

    defines "NEON_ASSET_MGR_DISABLE_HASH_VALIDATION"

    links
    {
        "NeonCore",
        "NeonCoroutines"
    }

    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Coroutines}",
        "%{CommonDir.Neon.Resource}"
    }