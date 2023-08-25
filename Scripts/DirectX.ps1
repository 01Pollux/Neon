Push-Location -Path .\tmp -StackName "TempPath"

Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.610.5 -OutFile Agility.zip
Expand-Archive Agility.zip -DestinationPath Agility
Remove-Item Agility.zip

Pop-Location -StackName "TempPath"

#

Write-Output "Copying DirectX Agility SDK libraries..."
Remove-Item -Force -Recurse .\Deps\Libs\DxAgility -ErrorAction SilentlyContinue
New-Item .\Deps\Libs\DxAgility -Type Directory | Out-Null
Copy-Item -Force -Recurse .\tmp\Agility\build\native\bin\* .\Deps\Libs\DxAgility

Write-Output "Clearing DirectX Agility SDK files..."
Remove-Item tmp\Agility -Recurse -Force
Remove-Item -Force -Recurse .\packages -ErrorAction SilentlyContinue
