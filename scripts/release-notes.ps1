param(
    [Parameter(Mandatory = $true)][string]$Version,
    [string]$ChangelogPath = (Join-Path (Split-Path -Parent $PSScriptRoot) 'CHANGELOG.md'),
    [string]$OutputPath
)
$ErrorActionPreference = 'Stop'
$number = $Version -replace '^v', ''
if ($number -notmatch '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$') {
    throw 'A release needs a stable version such as 2.1.0 or v2.1.0.'
}
$source = Get-Content -LiteralPath $ChangelogPath -Raw -Encoding UTF8
$escaped = [regex]::Escape($number)
$versions = [regex]::Matches($source, "(?ms)^## \[$escaped\][^\r\n]*\r?\n(.*?)(?=^## \[|\z)")
if ($versions.Count -ne 1) { throw "Expected one changelog section for $number." }
$body = $versions[0].Groups[1].Value.Trim()
$sections = [regex]::Matches($body, '(?ms)^[ \t]*<!-- gpm:lang (en|zh-TW|zh-CN|shared) -->[ \t]*\r?\n(.*?)^[ \t]*<!-- gpm:endlang -->[ \t]*\r?$')
$languages = @{}
foreach ($section in $sections) {
    $language = $section.Groups[1].Value
    $content = $section.Groups[2].Value.Trim()
    if ($languages.ContainsKey($language) -or !$content -or $content.Contains('<!-- gpm:lang')) {
        throw "Invalid or repeated $language block in $number."
    }
    $languages[$language] = $content
}
foreach ($language in @('en', 'zh-TW', 'zh-CN')) {
    if (!$languages.ContainsKey($language)) { throw "Missing $language release notes for $number." }
}
if ([regex]::Matches($body, '<!-- gpm:lang').Count -ne $sections.Count -or
    [regex]::Matches($body, '<!-- gpm:endlang -->').Count -ne $sections.Count) {
    throw "Unmatched or unsupported language markers in $number."
}
$notes = "# GoPieMenu $number`n`n$body`n"
if ($OutputPath) {
    $absolute = [IO.Path]::GetFullPath($OutputPath)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($absolute)) | Out-Null
    [IO.File]::WriteAllText($absolute, $notes, [Text.UTF8Encoding]::new($false))
} else {
    $notes
}
