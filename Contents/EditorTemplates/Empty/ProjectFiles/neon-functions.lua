function setup_csharp_project_impl()
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"

    targetdir ("%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}")
	objdir ("%{wks.location}/bin-int/%{cfg.buildcfg}/%{prj.name}")

    debugdir "$(targetdir)"

    files
	{
        "**.cs",
		"**.natvis"
	}
end

function setup_csharp_project()
	setup_csharp_project_impl()
    links
    {
        "../NeonEngineS"
	}
end