project "ZipLib"
	kind "StaticLib"
	language "C++"
	staticruntime "On"
	
	systemversion "latest"
	cppdialect "C++17"

	common_dir_setup()

	files
	{
		"*.cpp",
		"*.h",
		"utils/*.h",
		"streams/**.h",
		"methods/*.h",
		"detail/*.cpp",
		"detail/*.h",
		"compression/**.h",
		"bzip2/*.h",
		"bzip2/*.c",
		"lzma/*.h",
		"lzma/*.c",
		"zlib/*.h",
		"zlib/*.c",
	}

    links
    {
        "bzip2",
        "zlib",
        "lzma"
    }

    disablewarnings
    {
		"4244",
        "4267",
        "4996"
    }

	include "Deps/Externals/ZipLib/extlibs/bzip2"
	include "Deps/Externals/ZipLib/extlibs/lzma"
	include "Deps/Externals/ZipLib/extlibs/zlib"