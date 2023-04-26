
OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

CommonDir = {}
CommonDir["Deps"] = {}
CommonDir["Neon"] = {}

CommonDir["Deps"]["Inc"] = "%{wks.location}/Deps/Public"
CommonDir["Deps"]["Libs"] = "%{wks.location}/Deps/Libs"
CommonDir["Neon"]["Core"] = "%{wks.location}/Neon/Core/Public"
CommonDir["Neon"]["Engine"] = "%{wks.location}/Neon/Engine/Public"
CommonDir["Neon"]["Windowing"] = "%{wks.location}/Neon/Windowing/Public"
CommonDir["Neon"]["Graphics"] = "%{wks.location}/Neon/Graphics/Public"
