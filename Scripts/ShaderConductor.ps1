Push-Location -Path .\tmp -StackName "TempPath"

Invoke-WebRequest -Uri https://github.com/microsoft/ShaderConductor/releases/download/v0.3.0/ShaderConductor-win-vc142-x64-Release.zip -OutFile ShaderConductor.zip
Expand-Archive ShaderConductor.zip -DestinationPath ShaderConductor
Remove-Item ShaderConductor.zip

Pop-Location -StackName "TempPath"

#

Write-Output "Copying ShaderConductor libraries..."

Remove-Item -Force -Recurse .\Deps\Libs\ShaderConductor -ErrorAction SilentlyContinue
New-Item .\Deps\Libs\ShaderConductor -Type Directory | Out-Null
Copy-Item -Force -Recurse .\tmp\ShaderConductor\*\Bin\* -Exclude *.exe .\Deps\Libs\ShaderConductor
Copy-Item -Force -Recurse .\tmp\ShaderConductor\*\Lib\* .\Deps\Libs\ShaderConductor

Remove-Item -Force -Recurse .\Deps\Public\ShaderConductor -ErrorAction SilentlyContinue
Copy-Item -Force -Recurse .\tmp\ShaderConductor\*\Include\* .\Deps\Public