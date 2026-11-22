$ErrorActionPreference = 'Stop'

$root     = Split-Path -Parent $PSScriptRoot
$premake  = Join-Path $PSScriptRoot 'premake\premake5.exe'
$solution = Join-Path $root 'eftal.sln'
$vswhere  = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

function Invoke-Premake {
    & $premake --file="$root\premake5.lua" vs2022
    return $LASTEXITCODE -eq 0
}

# .lua edits change project settings; adding/removing/renaming files under src/ changes the globbed file lists.
# Submodules under ThirdParty/ are ignored, their own files never affect our projects.
function Test-Relevant([string]$path, [string]$change) {
    $rel = $path.Substring($root.Length + 1)
    if ($rel -match '^ThirdParty\\[^\\]+\\') { return $false }
    if ($rel -like '*.lua') { return $true }
    return ($rel -match '^[^\\]+\\src\\') -and ($change -ne 'Changed')
}

if (-not (Invoke-Premake)) {
    Read-Host 'premake failed, press Enter to exit'
    exit 1
}

$devenv = & $vswhere -version '[17.0,18.0)' -latest -property productPath
$vs = Start-Process $devenv -ArgumentList "`"$solution`"" -PassThru

$watcher = [IO.FileSystemWatcher]::new($root)
$watcher.IncludeSubdirectories = $true
$watcher.NotifyFilter = 'FileName, DirectoryName, LastWrite'
foreach ($evt in 'Changed', 'Created', 'Deleted', 'Renamed') {
    Register-ObjectEvent $watcher $evt -SourceIdentifier "eftal.$evt" | Out-Null
}
$watcher.EnableRaisingEvents = $true

Write-Host 'Watching for project changes. Closes when Visual Studio exits.'

while (-not $vs.HasExited) {
    if (-not (Wait-Event -Timeout 1)) { continue }

    # Editors often save as several events (temp file, rename); let them settle into one regenerate.
    Start-Sleep -Milliseconds 200

    $dirty = $false
    foreach ($e in @(Get-Event)) {
        if (Test-Relevant $e.SourceEventArgs.FullPath $e.SourceEventArgs.ChangeType) { $dirty = $true }
        Remove-Event -EventIdentifier $e.EventIdentifier
    }

    if ($dirty) {
        if (-not (Invoke-Premake)) { Write-Warning 'premake failed, fix the script and save again' }
    }
}
