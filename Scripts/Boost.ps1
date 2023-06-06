Push-Location -Path .\tmp -StackName "TempPath"

git clone --recursive https://github.com/boostorg/boost.git

Push-Location -Path .\boost -StackName "BoostTempPath"

.\bootstrap.bat
.\b2 address-sanitizer=norecover undefined-sanitizer=on runtime-link=static threading=multi -s BZIP2_SOURCE=..\..\Deps\Externals\bzip2 -s ZLIB_SOURCE=..\..\Deps\Externals\zlib

Pop-Location -StackName "BoostTempPath"
Pop-Location -StackName "TempPath"

#

Write-Output "Copying boost headers..."

Remove-Item -Force -Recurse .\Deps\Public\boost -ErrorAction SilentlyContinue
New-Item .\Deps\Public\boost -Type Directory | Out-Null
Copy-Item -Force -Recurse .\tmp\boost\boost\* .\Deps\Public\boost


Write-Output "Copying boost libraries..."

Remove-Item -Force -Recurse .\Deps\Libs\boost -ErrorAction SilentlyContinue
New-Item .\Deps\Libs\boost -Type Directory | Out-Null
Copy-Item -Recurse.\tmp\boost\stage\lib\*.lib .\Deps\Libs\boost


Write-Output "Clearing boost files..."
Remove-Item tmp\boost -Recurse -Force
