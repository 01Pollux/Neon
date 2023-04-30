.\Scripts\Boost.ps1

Function Get-DepSrcPath
{
    Param([string]$LibName)
    return ".\Deps\Externals\" + $LibName;
}

Function Get-DepLibPath
{
    Param([string]$LibName)
    return ".\Deps\Libs\" + $LibName;
}

Function Get-DepIncPath
{
    Param([string]$LibName)
    return ".\Deps\Public\" + $LibName;
}

Function Copy-IncludePathAbs
{
    Param
    (
        [Parameter(Mandatory=$true)][string]$SrcPath,
        [Parameter(Mandatory=$true)][string]$DstPath
    )
    New-Item -ItemType File -Path $DstPath -Force | Out-Null
    Copy-Item -Recurse $SrcPath $DstPath -Force -ErrorVariable CapturedErrors -ErrorAction SilentlyContinue
    $CapturedErrors | foreach-object { if ($_ -notmatch "already exists") { write-error $_ } }
}

Function Copy-IncludePath
{
    Param
    (
        [Parameter(Mandatory=$true)][string]$SrcPath,
        [Parameter(Mandatory=$true)][string]$DstPath,
        [switch]$Recurse
    )
    Copy-Item -Recurse:$Recurse $(Get-DepSrcPath($SrcPath)) $(Get-DepIncPath($DstPath)) -ErrorVariable CapturedErrors -ErrorAction SilentlyContinue
    $CapturedErrors | foreach-object { if ($_ -notmatch "already exists") { write-error $_ } }
}

Function Remove-Directory
{
    Param([string]$Path)
    Remove-Item $Path -Force -Recurse -ErrorAction SilentlyContinue
}

Function Make-Directory
{
    Param([string]$Path)
    New-Item -ItemType Directory $Path -Force -ErrorVariable CapturedErrors -ErrorAction SilentlyContinue | Out-Null
    $CapturedErrors | foreach-object { if ($_ -notmatch "already exists") { write-error $_ } }
}



# Initialize premake + include files


#
# flecs
#
Write-Output "Copying flecs files..."
Copy-Item Premake\flecs.lua $(Get-DepSrcPath("flecs\premake5.lua"))

Remove-Directory $(Get-DepIncPath("flecs"))
Copy-Item -Recurse $(Get-DepSrcPath("flecs\include")) $(Get-DepIncPath("flecs"))


#
# ImGui
#
Write-Output "Copying ImGui files..."
Remove-Directory $(Get-DepIncPath("ImGui"))
Make-Directory $(Get-DepIncPath("ImGui\backends"))
Make-Directory $(Get-DepIncPath("ImGui\misc"))

Copy-Item Premake\ImGui.lua $(Get-DepSrcPath("ImGui\premake5.lua"))

Copy-IncludePath "ImGui\backends\*.h" "ImGui\backends" -Recurse
Copy-IncludePath "ImGui\*.h" "ImGui"
Copy-IncludePath "ImGui\misc" "ImGui" -Recurse


#
# LibOgg
#
Write-Output "Copying LibOgg files..."
Remove-Directory $(Get-DepIncPath("LibOgg"))
Make-Directory $(Get-DepIncPath("LibOgg\ogg"))

Copy-Item Premake\LibOgg.lua $(Get-DepSrcPath("libogg-1.3.0\premake5.lua"))

Copy-IncludePath "libogg-1.3.0\include\ogg\*.h" "LibOgg\ogg"
Copy-IncludePath "libogg-1.3.0\include\ogg\*.h" "LibOgg"


#
# Vorbis
#
Write-Output "Copying Vorbis files..."
Remove-Directory $(Get-DepIncPath("Vorbis"))
Make-Directory $(Get-DepIncPath("Vorbis"))

Copy-Item Premake\LibVorbis.lua $(Get-DepSrcPath("vorbis\premake5.lua"))

Copy-IncludePath "vorbis\include\vorbis\*.h" "Vorbis"


#
# ZipLib
#
Write-Output "Copying ZipLib files..."
Remove-Directory $(Get-DepIncPath("ZipLib"))
Make-Directory $(Get-DepIncPath("ZipLib"))

Copy-Item Premake\ZipLib\ZipLib.lua $(Get-DepSrcPath("ZipLib\premake5.lua"))
Copy-Item Premake\ZipLib\bzip2.lua $(Get-DepSrcPath("ZipLib\extlibs\bzip2\premake5.lua"))
Copy-Item Premake\ZipLib\lzma.lua $(Get-DepSrcPath("ZipLib\extlibs\lzma\premake5.lua"))
Copy-Item Premake\ZipLib\zlib.lua $(Get-DepSrcPath("ZipLib\extlibs\zlib\premake5.lua"))

Copy-Item $(Get-DepSrcPath("ZipLib\*.h")) $(Get-DepIncPath("ZipLib"))
Copy-Item $(Get-DepSrcPath("ZipLib\utils")) $(Get-DepIncPath("ZipLib\utils")) -Recurse
Copy-Item $(Get-DepSrcPath("ZipLib\streams")) $(Get-DepIncPath("ZipLib\streams")) -Recurse
Copy-Item $(Get-DepSrcPath("ZipLib\methods")) $(Get-DepIncPath("ZipLib\methods")) -Recurse
Copy-Item $(Get-DepSrcPath("ZipLib\compression")) $(Get-DepIncPath("ZipLib\compression")) -Recurse
Copy-Item $(Get-DepSrcPath("ZipLib\extlibs\zlib")) $(Get-DepIncPath("ZipLib\extlibs\zlib")) -Recurse
Copy-Item $(Get-DepSrcPath("ZipLib\extlibs\lzma")) $(Get-DepIncPath("ZipLib\extlibs\lzma")) -Recurse
Copy-Item $(Get-DepSrcPath("ZipLib\extlibs\bzip2")) $(Get-DepIncPath("ZipLib\extlibs\bzip2")) -Recurse


#
# Bullet3
#
Write-Output "Copying Bullet3 files..."
Remove-Directory $(Get-DepIncPath("Bullet3"))
Make-Directory $(Get-DepIncPath("Bullet3"))

Copy-Item Premake\Bullet3\Buller3.lua $(Get-DepSrcPath("Bullet3\src\premake5.lua"))
Copy-Item Premake\Bullet3\Bullet3Common.lua $(Get-DepSrcPath("Bullet3\src\Bullet3Common\premake5.lua"))
Copy-Item Premake\Bullet3\BulletCollision.lua $(Get-DepSrcPath("Bullet3\src\BulletCollision\premake5.lua"))
Copy-Item Premake\Bullet3\BulletDynamics.lua $(Get-DepSrcPath("Bullet3\src\BulletDynamics\premake5.lua"))
Copy-Item Premake\Bullet3\BulletInverseDynamics.lua $(Get-DepSrcPath("Bullet3\src\BulletInverseDynamics\premake5.lua"))
Copy-Item Premake\Bullet3\BulletSoftBody.lua $(Get-DepSrcPath("Bullet3\src\BulletSoftBody\premake5.lua"))
Copy-Item Premake\Bullet3\LinearMath.lua $(Get-DepSrcPath("Bullet3\src\LinearMath\premake5.lua"))


#
# spdlog
#
Write-Output "Copying spdlog files..."
Remove-Directory $(Get-DepIncPath("spdlog"))
Make-Directory $(Get-DepIncPath("spdlog"))

Copy-Item Premake\spdlog.lua $(Get-DepSrcPath("spdlog\premake5.lua"))

Copy-IncludePath "spdlog\include\spdlog" "." -Recurse


$SrcPath = $(Get-DepSrcPath("Bullet3\src"))
Get-ChildItem $SrcPath -Recurse -Filter "*.h" |
Foreach {
    $Path = "Bullet3\" + ($_.FullName -Replace "^.*Bullet3\\src\\", "")
    Copy-IncludePathAbs $_.FullName $(Get-DepIncPath($Path))
}

#
# Dxc
#
Write-Output "Copying DxC files..."
Copy-Item .\Vendors\DxC .\Deps\Libs -Force -Recurse