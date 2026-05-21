param(
    [string]$Executable = "build\Debug\UnitConverter.exe",
    [string]$Expected = "tests\golden_master_expected.txt",
    [switch]$SkipGitAdd
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    if (-not (Test-Path $Executable)) {
        throw "UnitConverter executable was not found: $Executable. Build the project first."
    }

    $exePath = (Resolve-Path $Executable).Path
    $expectedPath = Join-Path $projectRoot $Expected
    $expectedDirectory = Split-Path -Parent $expectedPath
    New-Item -ItemType Directory -Force $expectedDirectory | Out-Null

    $scenarios = @(
        "meter:2.5",
        "feet:1.0",
        "yard:1.0",
        "meter:0.0"
    )

    $tempDirectory = Join-Path $projectRoot "build\golden_master"
    New-Item -ItemType Directory -Force $tempDirectory | Out-Null

    $sections = New-Object System.Collections.Generic.List[string]
    for ($index = 0; $index -lt $scenarios.Count; $index++) {
        $scenario = $scenarios[$index]
        $inputPath = Join-Path $tempDirectory "input_$index.txt"
        $actualPath = Join-Path $tempDirectory "actual_$index.txt"

        Set-Content -Path $inputPath -Value $scenario -Encoding ascii

        $command = "`"$exePath`" < `"$inputPath`" > `"$actualPath`" 2> NUL"
        & cmd.exe /d /c $command
        if ($LASTEXITCODE -ne 0) {
            throw "UnitConverter failed for scenario: $scenario"
        }

        $actual = Get-Content -Path $actualPath -Raw
        $sections.Add("[$scenario]`n$actual")
    }

    $content = [string]::Join("---`n", $sections)
    Set-Content -Path $expectedPath -Value $content -NoNewline -Encoding ascii

    if (-not $SkipGitAdd) {
        git add -- $Expected
        if ($LASTEXITCODE -ne 0) {
            throw "git add failed for: $Expected"
        }
    }

    Write-Host "Generated Golden Master: $Expected"
} finally {
    Pop-Location
}
