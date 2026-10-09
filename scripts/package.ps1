param(
    [string]$QtRoot = $env:QT_ROOT_DIR,
    [string]$BuildDir = "build",
    [string]$InnoCompiler = "${env:ProgramFiles(x86)}/Inno Setup 6/ISCC.exe"
)
$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$originalPath = $env:PATH
Push-Location $repoRoot
try {
    $cmakeVersion = [regex]::Match((Get-Content CMakeLists.txt -Raw), 'project\(GoPieMenu VERSION ([0-9.]+)').Groups[1].Value
    $installerVersion = [regex]::Match((Get-Content installer.iss -Raw), '(?m)^AppVersion=([0-9.]+)').Groups[1].Value
    if (!$cmakeVersion -or $cmakeVersion -ne $installerVersion) { throw "Application and installer versions must match." }
    if (!(Test-Path -LiteralPath $InnoCompiler -PathType Leaf)) { throw "Inno Setup compiler not found: $InnoCompiler" }
    if ($QtRoot) { $env:PATH = (Join-Path $QtRoot "bin") + ";" + $env:PATH }
    $deployTool = (Get-Command windeployqt -ErrorAction Stop).Source
    New-Item -ItemType Directory -Force deploy | Out-Null
    Copy-Item -LiteralPath (Join-Path $BuildDir "Release/GoPieMenu.exe") -Destination deploy/GoPieMenu.exe
    Copy-Item -LiteralPath LICENSE,THIRD_PARTY_LICENSES.md -Destination deploy
    & $deployTool --release --qmldir src/ui/qml --no-translations deploy/GoPieMenu.exe
    if ($LASTEXITCODE) { throw "Qt deployment failed." }

    $env:PATH = "$env:SystemRoot/System32;$env:SystemRoot"
    $smokeProcess = Start-Process -FilePath ./deploy/GoPieMenu.exe -ArgumentList --smoke-test -WindowStyle Hidden -PassThru
    if (!$smokeProcess.WaitForExit(15000)) { $smokeProcess.Kill(); throw "Startup smoke test timed out." }
    if ($smokeProcess.ExitCode) { throw "Startup smoke test failed: $($smokeProcess.ExitCode)" }
    Write-Output "SDK-independent startup smoke test passed."

    & $InnoCompiler /Qp installer.iss
    if ($LASTEXITCODE) { throw "Installer compilation failed." }
    $hash = (Get-FileHash -LiteralPath Output/GoPieMenu_Setup.exe -Algorithm SHA256).Hash.ToLowerInvariant()
    "$hash  GoPieMenu_Setup.exe" | Set-Content -LiteralPath Output/SHA256SUMS.txt -Encoding ascii
    Write-Output "Packaged GoPieMenu $cmakeVersion with SHA-256 $hash"
} finally {
    $env:PATH = $originalPath
    Pop-Location
}
