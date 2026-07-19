# Zenithium - Windows installer build helper.
#
# Usage:
#     .\packaging\windows\build-installer.ps1
#
# Steps:
#     1. Configure + build a Release with the windows-msvc-release preset.
#     2. Install into build/windows-msvc-release/dist/ (windeployqt runs here).
#     3. Run Inno Setup (iscc) against Zenithium.iss.
#
# Requires: MSVC (any recent VS install), CMake, Ninja, and Inno Setup 6 or 7
# (iscc.exe on PATH or in the default Program Files location).

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

    # ---- 1. Locate + import MSVC env ------------------------------------
    $vcvars = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $vcvars)) {
        $found = Get-ChildItem "C:\Program Files*\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat" -ErrorAction SilentlyContinue |
                 Select-Object -First 1
        if ($found) { $vcvars = $found.FullName }
    }
    if (-not (Test-Path $vcvars)) {
        throw "Could not locate vcvars64.bat. Set it manually at the top of this script."
    }
    Write-Host ">>> Importing MSVC env from $vcvars"

    $quoted = '"' + $vcvars + '"'
    $envLines = & cmd.exe /c "$quoted >NUL 2>&1 & set"
    foreach ($line in $envLines) {
        if ($line -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }

    $buildDir = "build\$Preset"
    $stageDir = Join-Path $repoRoot "$buildDir\dist"

    # ---- 2. Locate Qt6 --------------------------------------------------
    if (-not $env:CMAKE_PREFIX_PATH -and -not $env:Qt6_DIR) {
        $qtRoots = @()
        foreach ($base in @("C:\Qt", "D:\Qt")) {
            if (Test-Path $base) {
                $qtRoots += Get-ChildItem $base -Directory -ErrorAction SilentlyContinue |
                    Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' } |
                    Sort-Object Name -Descending
            }
        }
        foreach ($qtDir in $qtRoots) {
            $candidate = Join-Path $qtDir.FullName "msvc2019_64\lib\cmake\Qt6"
            if (Test-Path $candidate) {
                Write-Host ">>> Found Qt6 at $candidate"
                $env:Qt6_DIR = $candidate
                $env:CMAKE_PREFIX_PATH = Join-Path $qtDir.FullName "msvc2019_64"
                break
            }
        }
        if (-not $env:Qt6_DIR) {
            throw "Could not locate Qt6. Set Qt6_DIR or CMAKE_PREFIX_PATH before running."
        }
    }

    # ---- 3. Configure + build -------------------------------------------
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
            "C:\Program Files (x86)\Inno Setup 7\iscc.exe",
            "C:\Program Files\Inno Setup 7\iscc.exe",
            "C:\Program Files (x86)\Inno Setup 6\iscc.exe",
            "C:\Program Files\Inno Setup 6\iscc.exe"
        )) {
            if (Test-Path $cand) { $iscc = @{ Source = $cand }; break }
        }
    }
    if (-not $iscc) {
        throw "iscc.exe not found. Install Inno Setup 6 or 7 (https://jrsoftware.org/isdl.php)."
    }

    $iss = Join-Path $PSScriptRoot "Zenithium.iss"
    Write-Host ">>> Running iscc" -ForegroundColor Cyan
    & $iscc.Source "/DAppVersion=$Version" "/DStageDir=$stageDir" $iss
    if ($LASTEXITCODE -ne 0) { throw "iscc failed" }

    $outDir = Join-Path $repoRoot "dist-installer"
    Write-Host ""
    Write-Host ">>> Installer written to $outDir" -ForegroundColor Green
    Get-ChildItem $outDir -Filter "Zenithium-Setup-*.exe" |
        ForEach-Object { Write-Host ("    " + $_.FullName) }
}
finally {
    Pop-Location
}
