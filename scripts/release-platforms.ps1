param(
    [Parameter(Mandatory = $true)][string]$Version,
    [string]$ChangelogPath = (Join-Path (Split-Path -Parent $PSScriptRoot) 'CHANGELOG.md'),
    [string]$CMakePath = (Join-Path (Split-Path -Parent $PSScriptRoot) 'CMakeLists.txt'),
    [string]$ProjectVersion,
    [string]$OutputPath
)
$ErrorActionPreference = 'Stop'
$number = $Version -replace '^v', ''
if ($number -notmatch '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$') {
    throw 'A release needs a stable version such as 2.4.0 or v2.4.0.'
}
if (!$ProjectVersion) {
    if (!(Test-Path -LiteralPath $CMakePath -PathType Leaf)) { throw "CMakeLists.txt not found: $CMakePath" }
    $ProjectVersion = [regex]::Match((Get-Content -LiteralPath $CMakePath -Raw), 'project\(GoPieMenu VERSION ([0-9.]+)').Groups[1].Value
}
if ($ProjectVersion -ne $number) {
    throw "The release version $number does not match the application version $ProjectVersion."
}

$source = Get-Content -LiteralPath $ChangelogPath -Raw -Encoding UTF8
$escaped = [regex]::Escape($number)
$sections = [regex]::Matches($source, "(?ms)^## \[$escaped\][^\r\n]*\r?\n(.*?)(?=^## \[|\z)")
if ($sections.Count -ne 1) { throw "Expected one changelog section for $number." }
$body = $sections[0].Groups[1].Value

# Each version section declares the platforms whose users receive this release.
# The marker stays out of the language blocks so release notes keep their format.
$markers = [regex]::Matches($body, '(?m)^[ \t]*<!-- gpm:platforms[ \t]+([a-z,\s]+?)[ \t]*-->[ \t]*\r?$')
if ($markers.Count -ne 1) { throw "The $number section needs exactly one gpm:platforms marker." }

$platforms = @()
foreach ($entry in $markers[0].Groups[1].Value -split ',') {
    $platform = $entry.Trim().ToLowerInvariant()
    if (!$platform) { throw "The $number gpm:platforms marker contains an empty entry." }
    if ($platform -notin @('windows', 'macos')) { throw "Unknown platform '$platform' in the $number release." }
    if ($platforms -contains $platform) { throw "Duplicate platform '$platform' in the $number release." }
    $platforms += $platform
}
if ($platforms.Count -eq 0) { throw "The $number gpm:platforms marker lists no platforms." }

# GitHub's "latest" badge and the /releases/latest endpoint only exist for clients
# released before the per-platform channel. Those clients are Windows builds, so a
# release stays "latest" only when Windows receives it and keeps old clients from
# seeing a macOS-only release.
$latest = $platforms -contains 'windows'
$result = [ordered]@{
    version   = $number
    platforms = @($platforms)
    latest    = $latest
}
$json = ($result | ConvertTo-Json -Compress)
if ($OutputPath) {
    $absolute = [IO.Path]::GetFullPath($OutputPath)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($absolute)) | Out-Null
    [IO.File]::WriteAllText($absolute, $json + "`n", [Text.UTF8Encoding]::new($false))
}
$json
