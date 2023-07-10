Remove-Item bin-int -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item bin -Recurse -Force -ErrorAction SilentlyContinue

# Run premake
.\premake5.exe vs2022 GraphicsAPI=Directx12
