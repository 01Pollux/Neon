# Arguments: GenerateNAMD.ps1 -Guid <GUID> -Path <Path> LoaderId <LoaderId>
# Where Guid is optional and Path and LoaderId is required

param(
    [Parameter(Mandatory = $false)]
    [string]$Guid = [Guid]::NewGuid().ToString(),
    [Parameter(Mandatory = $true)]
    [string]$Path,
    [Parameter(Mandatory = $true)]
    [string]$LoaderId
)

$Hash = Get-FileHash -Path $Path -Algorithm SHA256 | Select-Object -ExpandProperty Hash;

$RelativePath = $Path;
if ($RelativePath.StartsWith(".\")) {
    $RelativePath = $RelativePath.Substring(2);
}

if ($RelativePath.StartsWith("Editor\")) {
    $RelativePath = $RelativePath.Substring(7);
}

$RelativePath = $RelativePath.Replace("\", "/");

$Template = @"
{
    "Guid": "$Guid",
    "Path": "$RelativePath.namd",
    "Loader": "",
    "Dependencies": "",
    "LoaderId": "$LoaderId",
    "Hash": "$Hash"
}
"@;

$Template | Out-File "$Path.namd" -Encoding ascii;