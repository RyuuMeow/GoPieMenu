$ErrorActionPreference = 'Stop'
$extractor = Join-Path (Split-Path -Parent $PSScriptRoot) 'scripts/release-platforms.ps1'
$changelog = Join-Path (Split-Path -Parent $PSScriptRoot) 'CHANGELOG.md'

function Write-Fixture([string]$directory, [string]$platforms, [string]$version = '2.4.0', [bool]$duplicate = $false) {
    $marker = ''
    if ($platforms -ne '<none>') {
        $marker = "<!-- gpm:platforms $platforms -->`n"
        if ($duplicate) { $marker += "<!-- gpm:platforms $platforms -->`n" }
    }
    $text = "# Changelog`n## [Unreleased]`n`n## [$version]`n$marker<!-- gpm:lang en -->`nChanges.`n<!-- gpm:endlang -->`n"
    $path = Join-Path $directory 'CHANGELOG.md'
    [IO.File]::WriteAllText($path, $text, [Text.UTF8Encoding]::new($false))
    $path
}

function MustFail([scriptblock]$Operation) {
    $failed = $false
    try { & $Operation | Out-Null } catch { $failed = $true }
    if (!$failed) { throw 'Invalid release platform metadata was accepted.' }
}

$testDirectory = Join-Path ([IO.Path]::GetTempPath()) ('gpm-platforms-' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($testDirectory) | Out-Null
try {
    $fixture = Write-Fixture $testDirectory 'macos'
    $result = & $extractor -Version v2.4.0 -ChangelogPath $fixture -ProjectVersion 2.4.0
    $parsed = $result | ConvertFrom-Json
    if ($parsed.version -ne '2.4.0' -or $parsed.latest -ne $false) { throw 'macOS-only release must not become GitHub latest.' }
    if (($parsed.platforms -join ',') -ne 'macos') { throw 'macOS-only platforms were parsed incorrectly.' }

    $result = & $extractor -Version 2.4.0 -ChangelogPath (Write-Fixture $testDirectory 'windows,macos') -ProjectVersion 2.4.0
    $parsed = $result | ConvertFrom-Json
    if ($parsed.latest -ne $true) { throw 'A release including Windows must stay GitHub latest.' }
    if (($parsed.platforms -join ',') -ne 'windows,macos') { throw 'Combined platforms were parsed incorrectly.' }

    # The output file is what the release workflow consumes.
    $output = Join-Path $testDirectory 'platforms.json'
    & $extractor -Version 2.4.0 -ChangelogPath $fixture -ProjectVersion 2.4.0 -OutputPath $output | Out-Null
    if (!(Test-Path -LiteralPath $output)) { throw 'The platform summary file was not written.' }
    if ((Get-Content -LiteralPath $output -Raw | ConvertFrom-Json).version -ne '2.4.0') { throw 'The platform summary file is invalid.' }

    MustFail { & $extractor -Version 2.4.0 -ChangelogPath (Write-Fixture $testDirectory '<none>') -ProjectVersion 2.4.0 }
    MustFail { & $extractor -Version 2.4.0 -ChangelogPath (Write-Fixture $testDirectory 'macos' -duplicate $true) -ProjectVersion 2.4.0 }
    MustFail { & $extractor -Version 2.4.0 -ChangelogPath (Write-Fixture $testDirectory 'linux') -ProjectVersion 2.4.0 }
    MustFail { & $extractor -Version 2.4.0 -ChangelogPath (Write-Fixture $testDirectory 'windows,macos,windows') -ProjectVersion 2.4.0 }
    MustFail { & $extractor -Version 2.4.0 -ChangelogPath (Write-Fixture $testDirectory 'windows,,macos') -ProjectVersion 2.4.0 }
    MustFail { & $extractor -Version 2.4.0 -ChangelogPath (Write-Fixture $testDirectory 'macos') -ProjectVersion 2.5.0 }
    MustFail { & $extractor -Version '2.4.0-beta' -ChangelogPath $fixture -ProjectVersion '2.4.0-beta' }

    # The historical changelog has no platform markers; releases keep being rejected
    # until the release notes declare them explicitly.
    MustFail { & $extractor -Version v2.0.0 -ChangelogPath $changelog }
}
finally {
    Get-ChildItem -LiteralPath $testDirectory -File | ForEach-Object { Remove-Item -LiteralPath $_.FullName }
    Remove-Item -LiteralPath $testDirectory
}
Write-Output 'Release platform markers, latest policy, and failure cases passed.'
