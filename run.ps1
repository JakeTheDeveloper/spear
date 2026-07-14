$ErrorActionPreference = "Stop"

& "$PSScriptRoot/build.ps1"

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& "$PSScriptRoot/build/windows/game.exe"
