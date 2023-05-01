Remove-Item -Force -Recurse .\tmp -ErrorAction SilentlyContinue

New-Item .\tmp -Type Directory | Out-Null

.\Install.ps1

Write-Output "Clearing temporary files..."
Remove-Item -Force -Recurse tmp | Out-Null

.\Setup.ps1
