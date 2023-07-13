Function Copy-Boost {
    Param
    (
        [Parameter(Mandatory = $true)][string]$Tag,
        [Parameter(Mandatory = $true)][string]$Path
    )

    Write-Output "$Tag Copying boost headers..."

    Remove-Item -Force -Recurse .\Deps\Public\$Path\boost -ErrorAction SilentlyContinue
    New-Item .\Deps\Public\$Path\boost -Type Directory | Out-Null
    Copy-Item -Force -Recurse .\tmp\boost\boost\* .\Deps\Public\$Path\boost


    Write-Output "$Tag Copying boost libraries..."

    Remove-Item -Force -Recurse .\Deps\Libs\$Path\boost -ErrorAction SilentlyContinue
    New-Item .\Deps\Libs\$Path\boost -Type Directory | Out-Null
    Copy-Item -Recurse .\tmp\boost\stage\lib\*.lib .\Deps\Libs\$Path\boost
}

Push-Location -Path .\tmp -StackName "TempPath"

git clone --recursive https://github.com/boostorg/boost.git

Push-Location -Path .\boost -StackName "BoostTempPath"
.\bootstrap.bat

# Boost with address sanitizer

.\b2 architecture=x86 address-model=64 address-sanitizer=on debug-symbols=on runtime-link=static threading=multi -s BZIP2_SOURCE=..\..\Deps\Externals\bzip2 -s ZLIB_SOURCE=..\..\Deps\Externals\zlib

Pop-Location -StackName "BoostTempPath"
Pop-Location -StackName "TempPath"
Copy-Boost "[Sanitizer]" "boost_san"

# Boost without address sanitizer

Push-Location -Path .\tmp -StackName "TempPath"
Push-Location -Path .\boost -StackName "BoostTempPath"
.\b2 architecture=x86 address-model=64 runtime-link=static threading=multi -s BZIP2_SOURCE=..\..\Deps\Externals\bzip2 -s ZLIB_SOURCE=..\..\Deps\Externals\zlib

Pop-Location -StackName "BoostTempPath"
Pop-Location -StackName "TempPath"
Copy-Boost "[Default]" "boost_def"

#

Write-Output "Clearing boost files..."
Remove-Item tmp\boost -Recurse -Force
