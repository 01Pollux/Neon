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
        "NeonEngine",
        "d3d12",
        "dxgi",
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

    links
    {
        "Flecs"
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