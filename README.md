# Neon Game Engine Project.
* Modern C++.
* Easy usage.
* Appeal to different demographics
	* Beginners
	* Intermediate
	* Advanced
	* Developper
* Basic Features
	* Frameworks
		- [ ] Modules can be extended / overridden depending on what the user wants.
	* Graphics 
		- [x] DirectX12
		- [ ] Vulkan
		- [ ] Metal
	* Window
		- [x] Basic window 
		- [ ] Customized window
		- [ ] Editor
		- [ ] Dockable window
	- [ ] Audio
	* Input
		- [ ] Basic pooling
		- [ ] Input hooking
	* Math types
		* Vector
		* Matrix
		* Rect
		* Viewport
		* Color
	* Logging
		* Console logging
		* File logging
	* Event system
		* Private events
		- [ ] Public events
	* Scripting
		- [ ] C# Support
	* Concurrency
		- [ ] Thread pooling
		- [ ] Coroutines
	* Editor
		- [ ] Compose Assets
		- [ ] Prepare the runtime
	* Runtime
		- [ ] Run only on assets
		- [ ] Can Run without any assets
		- [ ] Extendable
	* Resource Management
		- [x] Async
		- [x] Customizable Loaders, Savers, Exporters, Importers
		- [x] Multiple packs and resource format
		- [ ] Resource dependecy resolution
	* Rendering
		- [ ] 2D
		- [ ] 3D
		- [ ] 2.5D
		* Render graph
			- [ ] Automatic resource management
			- [ ] Automatic nodes dependecy resolution
			- [ ] Recompiles only when needed (Dependency change)
		- [ ] GUI
		- [ ] Frustum culling
			* GPU Side
			* CPU Side
		- [ ] Ray tracing
			* GPU Side
			* CPU Side
		- [ ] Post process effects
		- [ ] Light management
	- [ ] Scene
		* Game objects
		* Components
		* ECS (Flecs)
	- [ ] Networking
	* Cross platform
		- [x] Windows
		- [ ] Linux
		- [ ] Mac
		- [ ] Mobile


## Layer of usage.
* Simple
	* Easy to use, Easy to understand.
	* Game loop won't be visible.
	* Everything is hidden in simple classes/functions when needed.
	* Customizable, user will write simple main function, hook up any needed callbacks and call it a day.
	* Self contained resource management.
* Advanced
	* Internal API is exposed.
	* User must provide a propper management of game loop.
	* User can choose what framework / module to enable / disable.
* Framework
Individual modules can be constructed / changed.
