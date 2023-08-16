function copy_file_to_target_dir(from_dir, folder, name)
    postbuildcommands
    {
        string.format('if not exist "$(targetdir)%s" mkdir "$(targetdir)%s"', folder, folder),
        string.format("{COPYFILE} %s/%s $(targetdir)%s%s", from_dir, name, folder, name)
    }
end
function copy_directory_to_target_dir(from_dir, to_dir)
    filter "system:windows"
        postbuildcommands
        {
    	    string.format("powershell rmdir -R $(targetdir)%s 2>nul", to_dir)
        }
    filter {}
    postbuildcommands
    {
        string.format('mkdir "$(targetdir)%s"', to_dir),
        string.format("{COPY} %s $(targetdir)%s", from_dir, to_dir)
    }
end

function copy_engine_resources(content_name)
    if content_name ~= nil then
        content_name = "%{wks.location}Contents/"..content_name;
        copy_directory_to_target_dir(content_name, "Contents/");
	end

    copy_file_to_target_dir("%{CommonDir.Deps.Libs}/DxC/bin/x64", "", "dxcompiler.dll")
    copy_file_to_target_dir("%{CommonDir.Deps.Libs}/DxC/bin/x64", "", "dxil.dll")
    copy_file_to_target_dir("%{CommonDir.Deps.Libs}/DxAgility/x64", "D3D12/", "D3D12Core.dll")
    copy_file_to_target_dir("%{CommonDir.Deps.Libs}/DxAgility/x64", "D3D12/", "D3D12SDKLayers.dll")
    filter "configurations:not Dist"
        copy_file_to_target_dir("%{CommonDir.Deps.Libs}/DxAgility/x64", "D3D12/", "D3D12Core.pdb")
        copy_file_to_target_dir("%{CommonDir.Deps.Libs}/DxAgility/x64", "D3D12/", "D3D12SDKLayers.pdb")
    filter {}
end

function common_dir_setup()
	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    debugdir "$(targetdir)"
    
    disablewarnings
    {
        "4250"
    }
end

function common_add_pch(pch_file)
    pchheader (pch_file..".hpp")
    pchsource (pch_file..".cpp")
end

function select_launch_kind()
    filter "configurations:Dist"
        kind "WindowedApp"
    filter {}
    filter "configurations:not Dist"
        kind "ConsoleApp"
    filter {}
end

function link_boost_inc()
    filter "configurations:Dist"
        includedirs
        {
            "%{CommonDir.Deps.Inc}/boost_def"
        }
    filter {}
    filter "configurations:not Dist"
        includedirs
        {
            "%{CommonDir.Deps.Inc}/boost_san"
        }
    filter {}
end

function link_boost_lib(lib_name)
    filter "configurations:Dist"
        links
        {
            "boost_def/boost/libboost_"..lib_name.."-vc143-mt-s-x64-1_83.lib"
        }
    filter {}
    filter "configurations:not Dist"
        links
        {
            "boost_san/boost/libboost_"..lib_name.."-vc143-mt-sgd-x64-1_83.lib"
        }
    filter {}
end

--

function link_engine_library_no_engine()
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
        "glfw",
        "Flecs",
        "spdlog",

        "NeonCore",
        "NeonCoroutines",
        "NeonResource",
        "NeonWindowing",
        "NeonGraphics",
    }

    includedirs
    {
        "%{CommonDir.Deps.Inc}/Bullet3",

        "%{CommonDir.Neon.Core}",
        "%{CommonDir.Neon.Coroutines}",
        "%{CommonDir.Neon.Windowing}",
        "%{CommonDir.Neon.Graphics}",
        "%{CommonDir.Neon.Resource}",
        "%{CommonDir.Neon.Engine}"
    }

    
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
end

function link_engine_library(with_engine)
    link_engine_library_no_engine()
    links
    {
        "NeonEngine"
    }
end

--

function common_neon_defines()
    defines "flecs_STATIC"
    defines "BT_USE_DOUBLE_PRECISION"
    defines "GLM_FORCE_LEFT_HANDED"
end

function common_neon_links()
    libdirs "%{CommonDir.Deps.Libs}"
    
    link_boost_inc()
end

function common_neon()
    common_neon_defines()
    common_neon_links()

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

function setup_runtime_engine(asset_path_name)
    language "C++"
    cppdialect "C++latest"
    staticruntime "On"
    architecture "x64"

    select_launch_kind()
    common_dir_setup()
    common_neon()

    link_engine_library()
    copy_engine_resources(asset_path_name)
end