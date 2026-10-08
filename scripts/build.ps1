param(
    [string]$QtRoot = $env:QT_ROOT_DIR,
    [string]$BuildDir = "build",
    [switch]$SkipTests
)
$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = [IO.Path]::GetFullPath((Join-Path $repoRoot $BuildDir))
$configure = @("-S", $repoRoot, "-B", $buildPath, "-G", "Visual Studio 17 2022", "-A", "x64", "-DBUILD_TESTING=ON")
if ($QtRoot) {
    $configure += "-DCMAKE_PREFIX_PATH=$QtRoot"
    $env:PATH = (Join-Path $QtRoot "bin") + ";" + $env:PATH
}
& cmake @configure
if ($LASTEXITCODE) { throw "CMake configuration failed." }
& cmake --build $buildPath --config Release --parallel 4
if ($LASTEXITCODE) { throw "Build failed." }
if (!$SkipTests) {
    & ctest --test-dir $buildPath -C Release --output-on-failure --output-junit "$buildPath/test-results.xml"
    if ($LASTEXITCODE) {
        Get-ChildItem -LiteralPath "$buildPath/artifacts" -Filter *.txt | ForEach-Object { Get-Content -LiteralPath $_.FullName }
        throw "Regression tests failed."
    }
}
