
OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["NeonDepsInc"] = "%{wks.location}/Deps/Public"
IncludeDir["NeonDepsLib"] = "%{wks.location}/Deps/Libs"
IncludeDir["NeonModule"] = "%{wks.location}/Neon-Module/Public"
