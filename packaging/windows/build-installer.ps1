# Zenithium — Windows installer build helper.
#
# Usage:
#     .\packaging\windows\build-installer.ps1
#
# Steps:
#     1. Configure + build a Release with the windows-msvc-release preset.
#     2. Install into build/windows-msvc-release/dist/ (windeployqt runs here).
#     3. Run Inno Setup (iscc) against Zenithium.iss.
#
# Requires: vcvars64.bat on your PATH via VS install, CMake, Ninja, and
# Inno Setup 6 (iscc.exe on PATH or at "C:\Program Files (x86)\Inno Setup 6\").

param(
    [string] $Preset  = "windows-msvc-release",
    [string] $Version = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Push-Location $repoRoot
try {
    if (-not $Version) {
        $ver = Select-String -Path "cmake\ZenithiumVersion.cmake" `
                              -Pattern 'set\(ZENITHIUM_VERSION\s+"?([\d\.]+)"?\)' `
                              -AllMatches |
               ForEach-Object { $_.Matches.Groups[1].Value } |
               Select-Object -First 1
        if (-not $ver) { $ver = "0.1.0" }
        $Version = $ver
    }
    Write-Host ">>> Zenithium $Version" -ForegroundColor Cyan

    # ---- 1. Import MSVC env into this shell -------------------------------
    $vcvars = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $vcvars)) {
        # Fallback: search common VS install locations.
        $found = Get-ChildItem "C:\Program Files*\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat" -ErrorAction SilentlyContinue |
                 Select-Object -First 1
        if ($found) { $vcvars = $found.FullName }
    }
    if (-not (Test-Path $vcvars)) {
        throw "Could not locate vcvars64.bat — set it manually at the top of this script."
    }
    Write-Host ">>> Importing MSVC env from $vcvars"
    cmd /c "`"$vcvars`" >NUL 2>&1 && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }

    $buildDir = "build\$Preset"
    $stageDir = Join-Path $repoRoot "$buildDir\dist"

    # ---- 2. Configure + build --------------------------------------------
    Write-Host ">>> Configuring ($Preset)" -ForegroundColor Cyan
    cmake --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }

    Write-Host ">>> Building" -ForegroundColor Cyan
    cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }

    # ---- 3. Install (stages Qt DLLs via windeployqt) ---------------------
    if (Test-Path $stageDir) { Remove-Item $stageDir -Recurse -Force }
    Write-Host ">>> Installing to $stageDir" -ForegroundColor Cyan
    cmake --install $buildDir --prefix $stageDir --component Runtime
    if ($LASTEXITCODE -ne 0) { throw "cmake install failed" }

    # ---- 4. Run Inno Setup ----------------------------------------------
    $iscc = Get-Command iscc.exe -ErrorAction SilentlyContinue
    if (-not $iscc) {
        foreach ($cand in @(
            "C:\Program Files (x86)\Inno Setup 6\iscc.exe",
            "C:\Program Files\Inno Setup 6\iscc.exe"
        )) {
            if (Test-Path $cand) { $iscc = @{ Source = $cand }; break }
        }
    }
    if (-not $iscc) {
        throw "iscc.exe not found. Install Inno Setup 6 (https://jrsoftware.org/isinfo.php)."
    }

    $iss = Join-Path $PSScriptRoot "Zenithium.iss"
    Write-Host ">>> Running iscc" -ForegroundColor Cyan
    & $iscc.Source "/DAppVersion=$Version" "/DStageDir=$stageDir" $iss
    if ($LASTEXITCODE -ne 0) { throw "iscc failed" }

    $outDir = Join-Path $repoRoot "dist-installer"
    Write-Host ""
    Write-Host ">>> Installer written to $outDir" -ForegroundColor Green
    Get-ChildItem $outDir -Filter "Zenithium-Setup-*.exe" |
        ForEach-Object { Write-Host "    $($_.FullName)" }
}
finally {
    Pop-Location
}
