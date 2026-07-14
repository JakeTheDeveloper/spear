$ErrorActionPreference = "Stop"

$BuildDir = "build/windows"
$ObjDir = "$BuildDir/obj"
$RaylibLib = "libs/windows/raylib.lib"
$Output = "$BuildDir/game.exe"
$WindowsKitsLibRoot = "C:/Program Files (x86)/Windows Kits/10/Lib"
$MsvcLibRoot = "C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Tools/MSVC"

New-Item -ItemType Directory -Force $BuildDir | Out-Null
New-Item -ItemType Directory -Force $ObjDir | Out-Null

$Sources = Get-ChildItem -File *.cpp | Where-Object {
    $_.Name -notlike ".*" -and $_.Name -notlike "#*#"
}
$Objects = @()
$NeedsLink = !(Test-Path $Output)

foreach ($Source in $Sources) {
    $Object = Join-Path $ObjDir "$($Source.BaseName).obj"
    $Objects += $Object

    if (!(Test-Path $Object) -or $Source.LastWriteTimeUtc -gt (Get-Item $Object).LastWriteTimeUtc) {
        clang++ -std=c++17 -fms-runtime-lib=dll -Iinclude -c $Source.FullName -o $Object

        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }

        $NeedsLink = $true
    }

    if ((Test-Path $Output) -and (Get-Item $Object).LastWriteTimeUtc -gt (Get-Item $Output).LastWriteTimeUtc) {
        $NeedsLink = $true
    }
}

if ((Test-Path $RaylibLib) -and (Test-Path $Output) -and (Get-Item $RaylibLib).LastWriteTimeUtc -gt (Get-Item $Output).LastWriteTimeUtc) {
    $NeedsLink = $true
}

if ($NeedsLink) {
    if (!(Test-Path $RaylibLib)) {
        Write-Error "Missing $RaylibLib. Put a static Windows raylib.lib there."
    }

    $WindowsSdk = Get-ChildItem $WindowsKitsLibRoot -Directory |
        Sort-Object Name -Descending |
        Select-Object -First 1

    $Msvc = Get-ChildItem $MsvcLibRoot -Directory |
        Sort-Object Name -Descending |
        Select-Object -First 1

    $WindowsSdkLib = Join-Path $WindowsSdk.FullName "um/x64"
    $UcrtLib = Join-Path $WindowsSdk.FullName "ucrt/x64"
    $MsvcLib = Join-Path $Msvc.FullName "lib/x64"

    foreach ($Path in @($WindowsSdkLib, $UcrtLib, $MsvcLib)) {
        if (!(Test-Path $Path)) {
            Write-Error "Missing compiler library path: $Path"
        }
    }

    $SystemLibs = @(
        (Join-Path $WindowsSdkLib "opengl32.lib"),
        (Join-Path $WindowsSdkLib "gdi32.lib"),
        (Join-Path $WindowsSdkLib "winmm.lib"),
        (Join-Path $WindowsSdkLib "user32.lib"),
        (Join-Path $WindowsSdkLib "shell32.lib"),
        (Join-Path $UcrtLib "ucrt.lib"),
        (Join-Path $MsvcLib "msvcrt.lib"),
        (Join-Path $MsvcLib "vcruntime.lib")
    )

    foreach ($Path in $SystemLibs) {
        if (!(Test-Path $Path)) {
            Write-Error "Missing Windows system library: $Path"
        }
    }

    clang++ $Objects -o $Output `
        -L $WindowsSdkLib `
        -L $UcrtLib `
        -L $MsvcLib `
        -Xlinker /NODEFAULTLIB:libucrt.lib `
        $RaylibLib `
        $SystemLibs

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
