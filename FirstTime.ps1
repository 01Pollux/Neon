Remove-Item -Force -Recurse .\tmp -ErrorAction SilentlyContinue

New-Item .\tmp -Type Directory | Out-Null

.\Scripts\Directx.ps1

Write-Output "Clearing temporary files..."
Remove-Item -Force -Recurse tmp | Out-Null

.\Install.ps1
.\Setup_d3d12.ps1
