# Builds the installer: collects dist/ then runs CPack (NSIS required)

$ProjectDir  = $PSScriptRoot
$BuildDir    = "$ProjectDir\cmake-build-release"
$CPackExe    = (Get-ChildItem "D:\Programming\CLion*" -Recurse -Filter "cpack.exe" -ErrorAction SilentlyContinue | Select-Object -First 1).FullName
$CMakeExe    = (Get-ChildItem "D:\Programming\CLion*" -Recurse -Filter "cmake.exe"  -ErrorAction SilentlyContinue | Select-Object -First 1).FullName
$PythonExe   = "C:\Python313\python.exe"
$MSYS2Gcc    = "C:\msys64\mingw64\bin\gcc.exe"

if (-not $CPackExe) { Write-Error "cpack.exe not found in CLion folder"; exit 1 }
if (-not $CMakeExe) { Write-Error "cmake.exe not found in CLion folder"; exit 1 }

Write-Host "=== Step 1: Configure Release build ===" -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
& $CMakeExe -S $ProjectDir -B $BuildDir `
    -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    "-DCMAKE_C_COMPILER=$MSYS2Gcc" `
    2>&1
if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed"; exit 1 }

Write-Host "=== Step 2: Build Release ===" -ForegroundColor Cyan
& $CMakeExe --build $BuildDir --target inventory 2>&1
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit 1 }

Write-Host "=== Step 3: Collect distribution files ===" -ForegroundColor Cyan
& $PythonExe "$ProjectDir\cmake\collect_dist.py" `
    "$BuildDir\inventory.exe" `
    "$ProjectDir\dist" 2>&1
if ($LASTEXITCODE -ne 0) { Write-Error "collect_dist.py failed"; exit 1 }

Write-Host "=== Step 4: Create NSIS installer ===" -ForegroundColor Cyan
Push-Location $BuildDir
& $CPackExe -G NSIS --config CPackConfig.cmake 2>&1
Pop-Location
if ($LASTEXITCODE -ne 0) { Write-Error "CPack failed. Is NSIS installed?"; exit 1 }

$Installer = Get-ChildItem "$BuildDir\*.exe" | Where-Object { $_.Name -like "Inventory-*" } | Select-Object -First 1
Write-Host ""
Write-Host "=== Installer ready: $($Installer.FullName) ===" -ForegroundColor Green
