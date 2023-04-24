project "LibVorbis"
	kind "StaticLib"
	language "C"
	compileas "C"
	staticruntime "On"
	
	systemversion "latest"

	common_dir_setup()

	files
	{
		"symbian/*.h",
		"include/vorbis/**.h",
		"lib/analysis.c",
        "lib/barkmel.c",
        "lib/bitrate.c",
        "lib/block.c",
        "lib/codebook.c",
        "lib/envelope.c",
        "lib/floor0.c",
        "lib/floor1.c",
        "lib/info.c",
        "lib/lookup.c",
        "lib/lpc.c",
        "lib/lsp.c",
        "lib/mapping0.c",
        "lib/mdct.c",
        "lib/psy.c",
        "lib/registry.c",
        "lib/res0.c",
        "lib/sharedbook.c",
        "lib/smallft.c",
        "lib/synthesis.c",
        "lib/vorbisenc.c",
        "lib/vorbisfile.c",
        "lib/window.c",
		"lib/**.h"
	}

    includedirs
	{
        "./include",
        "./lib",
        "%{CommonDir.Deps.Inc}/LibOgg"
	}

    disablewarnings
    {
        "4244",
        "4267",
        "4305",
        "4554",
        "4996"
    }