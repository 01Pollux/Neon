project "HelloWorld"
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"

    select_launch_kind()
    common_dir_setup()
    common_neon()

    link_engine_library()
    copy_engine_resources()