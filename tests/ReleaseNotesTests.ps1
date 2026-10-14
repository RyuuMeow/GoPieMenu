$ErrorActionPreference = 'Stop'
$extractor = Join-Path (Split-Path -Parent $PSScriptRoot) 'scripts/release-notes.ps1'
$notes = & $extractor -Version v2.0.0
if (!$notes.StartsWith('# GoPieMenu 2.0.0') -or $notes.Contains('## [Unreleased]')) { throw 'Incorrect version extracted.' }
foreach ($language in @('en', 'zh-TW', 'zh-CN', 'shared')) {
    if (!$notes.Contains("<!-- gpm:lang $language -->")) { throw "Missing language: $language" }
}
if (!$notes.Contains('rebuilds the menu editor') -or !$notes.Contains('SHA256SUMS.txt')) { throw 'Release content was lost.' }
function MustFail([scriptblock]$Operation) {
    $failed = $false
    try { & $Operation | Out-Null } catch { $failed = $true }
    if (!$failed) { throw 'Invalid changelog was accepted.' }
}
MustFail { & $extractor -Version 99.99.99 }
MustFail { & $extractor -Version '2.0.0-beta' }
$testDirectory = Join-Path ([IO.Path]::GetTempPath()) ('gpm-release-notes-' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($testDirectory) | Out-Null
$fixture = Join-Path $testDirectory 'CHANGELOG.md'
$output = Join-Path $testDirectory 'notes.md'
try {
    $valid = "# Changelog`n## [2.1.0]`n"
    foreach ($language in @('en', 'zh-TW', 'zh-CN', 'shared')) {
        $valid += "<!-- gpm:lang $language -->`nContent for $language.`n<!-- gpm:endlang -->`n"
    }
    [IO.File]::WriteAllText($fixture, $valid)
    & $extractor -Version 2.1.0 -ChangelogPath $fixture -OutputPath $output
    $result = [IO.File]::ReadAllText($output)
    if (!$result.Contains('Content for zh-TW.') -or !$result.Contains('Content for shared.')) { throw 'Missing output content.' }
    [IO.File]::WriteAllText($fixture, $valid.Replace('gpm:lang zh-CN', 'gpm:lang unsupported'))
    MustFail { & $extractor -Version 2.1.0 -ChangelogPath $fixture }
    [IO.File]::WriteAllText($fixture, $valid + "`n<!-- gpm:lang en -->`nDuplicate`n<!-- gpm:endlang -->")
    MustFail { & $extractor -Version 2.1.0 -ChangelogPath $fixture }
    [IO.File]::WriteAllText($fixture, $valid + "`n<!-- gpm:lang en -->`nUnclosed")
    MustFail { & $extractor -Version 2.1.0 -ChangelogPath $fixture }
    [IO.File]::WriteAllText($fixture, $valid + "`n" + $valid)
    MustFail { & $extractor -Version 2.1.0 -ChangelogPath $fixture }
} finally {
    # These are the two exact temporary files created above; no recursive cleanup.
    if (Test-Path -LiteralPath $fixture) { Remove-Item -LiteralPath $fixture }
    if (Test-Path -LiteralPath $output) { Remove-Item -LiteralPath $output }
    Remove-Item -LiteralPath $testDirectory
}
Write-Output 'Release note extraction, language validation, and failure cases passed.'
