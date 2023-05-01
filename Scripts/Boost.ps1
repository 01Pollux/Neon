Push-Location -Path .\tmp -StackName "TempPath"

git clone --recursive https://github.com/boostorg/boost.git

Push-Location -Path .\boost -StackName "BoostTempPath"

.\bootstrap.bat
.\b2 runtime-link=static threading=multi

Pop-Location -StackName "BoostTempPath"
Pop-Location -StackName "TempPath"

#

Remove-Item -Force -Recurse .\Deps\Public\boost -ErrorAction SilentlyContinue
New-Item .\Deps\Public\boost -Type Directory | Out-Null

Write-Output "Copying boost headers..."
Copy-Item -Force -Recurse .\tmp\boost\boost\* .\Deps\Public\boost

Write-Output "Copying boost libraries..."
Copy-Item -Recurse.\tmp\boost\stage\lib\*.lib .\Deps\Libs\boost

Write-Output "Clearing boost files..."
Remove-Item tmp\boost -Recurse -Force