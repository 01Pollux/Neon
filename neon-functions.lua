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

function link_engine_library()
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
        "NeonGraphics",

        "NeonEngine"
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
    defines "BT_USE_DOUBLE_PRECISION"
    defines "GLM_FORCE_LEFT_HANDED"

    libdirs "%{CommonDir.Deps.Libs}"
    
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