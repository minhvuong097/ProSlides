param(
    [string]$QtPath = $env:CMAKE_PREFIX_PATH,
    [switch]$WithFFmpeg,
    [switch]$WithNDI,
    [switch]$WithTests,
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

if (-not $QtPath) {
    Write-Error "Set -QtPath or CMAKE_PREFIX_PATH to your Qt 6.12 MSVC installation."
}

$args = @(
    "-S", ".",
    "-B", $BuildDir,
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_PREFIX_PATH=$QtPath",
    "-DPS_ENABLE_FFMPEG=$(if ($WithFFmpeg) { 'ON' } else { 'OFF' })",
    "-DPS_ENABLE_NDI=$(if ($WithNDI) { 'ON' } else { 'OFF' })",
    "-DPS_BUILD_TESTS=$(if ($WithTests) { 'ON' } else { 'OFF' })"
)

Write-Host "Configuring ProSlides..."
cmake @args
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Building ProSlides (Release)..."
cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($WithTests) {
    Write-Host "Running tests..."
    ctest --test-dir $BuildDir -C Release --output-on-failure
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

Write-Host "Done: $BuildDir\Release\ProSlides.exe"
