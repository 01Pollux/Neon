if (!(Test-Path -path ".\Deps\Libs\boost"))
{
    New-Item ".\Deps\Libs\boost" -Type Directory | Out-Null
}

cd Deps
#git clone --recursive https://github.com/boostorg/boost.git

cd boost
#.\bootstrap.bat
#.\b2 runtime-link=static threading=multi

cd ../..

Write-Output "Copying boost headers..."
#Copy-Item -Force -Recurse .\Deps\boost\boost .\Deps\Public

Write-Output "Copying boost libraries..."
#Copy-Item .\Deps\boost\stage\lib\*.lib .\Deps\Libs\boost -Recurse

Write-Output "Clearing boost files..."
Remove-Item .\Deps\boost -Recurse -Force