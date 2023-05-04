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

function link_boost_lib(lib_name)
    filter "configurations:Dist"
        links
        {
            "boost/libboost_"..lib_name.."-vc143-mt-s-x64-1_83.lib"
        }
    filter "configurations:not Dist"
        links
        {
            "boost/libboost_"..lib_name.."-vc143-mt-sgd-x64-1_83.lib"
        }
end

function common_neon()
    defines "flecs_STATIC"

    libdirs "%{CommonDir.Deps.Libs}"

    link_boost_lib("atomic")
    link_boost_lib("chrono")
    link_boost_lib("container")
    link_boost_lib("context")
    link_boost_lib("contract")
    link_boost_lib("coroutine")
    link_boost_lib("date_time")
    link_boost_lib("exception")
    link_boost_lib("fiber")
    link_boost_lib("filesystem")
    link_boost_lib("graph")
    link_boost_lib("iostreams")
    link_boost_lib("json")
    link_boost_lib("locale")
    link_boost_lib("math_c99")
    link_boost_lib("math_c99f")
    link_boost_lib("math_c99l")
    link_boost_lib("math_tr1")
    link_boost_lib("math_tr1f")
    link_boost_lib("math_tr1l")
    link_boost_lib("nowide")
    link_boost_lib("program_options")
    link_boost_lib("random")
    link_boost_lib("regex")
    link_boost_lib("serialization")
    link_boost_lib("system")
    link_boost_lib("thread")
    link_boost_lib("timer")
    link_boost_lib("type_erasure")
    link_boost_lib("url")
    link_boost_lib("wave")
    link_boost_lib("wserialization")
    link_boost_lib("zlib")
  
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