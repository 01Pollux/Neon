function copy_to_targetdir(from_dir, name)
    postbuildcommands
    {
        string.format("{COPYFILE} %s/%s $(targetdir)/%s", from_dir, name, name)
    }
end

function copyres_and_remove_extension(name)
    postbuildcommands
    {
        string.format("tar -a -C %%{wks.location}Resources -cf  $(targetdir)%s.zip *", name),
        string.format("{MOVE} $(targetdir)%s.zip $(targetdir)%s", name, name)
    }
end

function copy_engine_resources()
    copyres_and_remove_extension("Assets")

    copy_to_targetdir("%{CommonDir.Deps.Libs}/DxC/bin/x86_64", "dxcompiler.dll")
    copy_to_targetdir("%{CommonDir.Deps.Libs}/DxC/bin/x86_64", "dxil.dll")
end

function common_dir_setup()
	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    debugdir "$(targetdir)"
    
    filter "configurations: not Dist"
        sanitize
        {
            "Address"
        }
    filter {}

    disablewarnings
    {
        "4250"
    }
end

function common_add_pch(pch_file)
    pchheader (pch_file..".hpp")
    pchsource (pch_file..".cpp")
end

function link_engine_library()
    links
    {
        "NeonEngine"
    }
    includedirs
    {
        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Engine}",
    }
end

function select_launch_kind()
    filter "configurations:Dist"
        kind "WindowedApp"
    filter {}
    filter "configurations:not Dist"
        kind "ConsoleApp"
    filter {}
end

function common_neon()
    defines "flecs_STATIC"

    libdirs "%{CommonDir.Deps.Libs}"

    filter "configurations:Dist"
        links
        {
            "boost/libboost_atomic-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_chrono-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_container-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_context-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_contract-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_coroutine-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_date_time-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_exception-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_fiber-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_filesystem-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_graph-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_iostreams-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_json-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_locale-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_math_c99-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_math_c99f-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_math_c99l-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_math_tr1-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_math_tr1f-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_math_tr1l-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_nowide-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_program_options-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_python39-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_random-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_regex-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_serialization-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_system-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_thread-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_timer-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_type_erasure-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_unit_test_framework-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_url-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_wave-vc143-mt-s-x64-1_83.lib",
            "boost/libboost_wserialization-vc143-mt-s-x64-1_83.lib"
        }
    filter {}
    filter "configurations:not Dist"
        links
        {
            "boost/libboost_atomic-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_chrono-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_container-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_context-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_contract-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_coroutine-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_date_time-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_exception-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_fiber-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_filesystem-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_graph-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_iostreams-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_json-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_locale-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_math_c99-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_math_c99f-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_math_c99l-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_math_tr1-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_math_tr1f-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_math_tr1l-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_nowide-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_program_options-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_python39-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_random-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_regex-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_serialization-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_system-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_thread-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_timer-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_type_erasure-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_unit_test_framework-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_url-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_wave-vc143-mt-sgd-x64-1_83.lib",
            "boost/libboost_wserialization-vc143-mt-sgd-x64-1_83.lib"
        }
    filter {}

    links
    {
        "Flecs",
        "spdlog"
    }

    files
    {
        "**.cpp",
        "**.hpp",
        "**.natvis"
    }

    includedirs
    {
        "%{CommonDir.Deps.Inc}",
        "%{prj.location}"
    }
end