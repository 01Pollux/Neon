project "HelloWorld"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x86_64"

    select_launch_kind()
    common_dir_setup()

    files
    {
        "**.cpp",
        "**.hpp",
        "**.natvis"
    }

    includedirs
    {
        "%{CommonDir.Deps.Inc}",
        "%{CommonDir.Neon.Engine}",
        "%{prj.location}"
    }

    link_engine_library()
    copy_engine_resources()