project "NeonEditor"
    setup_runtime_engine("Editor")
    common_add_pch("EditorPCH")
    
    includedirs
    {
        "%{CommonDir.Neon.Editor}"
    }