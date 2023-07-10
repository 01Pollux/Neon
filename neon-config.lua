
newoption {
	trigger = "GraphicsAPI",
	value = "API",
	description = "Choose a particular 3D API for rendering",
	allowed = {
		{ "Directx12",  "Direct3D 12 (Windows only)" }
	},
	default = "Directx12"
}

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

CommonDir = {}
CommonDir["Deps"] = {}
CommonDir["Neon"] = {}

CommonDir["Deps"]["Inc"] = "%{wks.location}/Deps/Public"
CommonDir["Deps"]["Libs"] = "%{wks.location}/Deps/Libs"
CommonDir["Neon"]["Core"] = "%{wks.location}/Neon/Core/Public"
CommonDir["Neon"]["Resource"] = "%{wks.location}/Neon/Resource/Public"
CommonDir["Neon"]["Engine"] = "%{wks.location}/Neon/Engine/Public"
CommonDir["Neon"]["Windowing"] = "%{wks.location}/Neon/Windowing/Public"
CommonDir["Neon"]["Graphics"] = "%{wks.location}/Neon/Graphics/Public"
CommonDir["Neon"]["Rendering"] = "%{wks.location}/Neon/Rendering/Public"
CommonDir["Neon"]["Coroutines"] = "%{wks.location}/Neon/Coroutines/Public"
