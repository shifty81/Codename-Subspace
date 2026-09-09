Set-StrictMode -Version 2

function Get-UniversalTreeStageFullPath {
    param([Parameter(Mandatory=$true)][string]$Path)
    return [System.IO.Path]::GetFullPath($Path)
}

function Get-UniversalTreeStageRelativePath {
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$Path
    )
    $rootFull = (Get-UniversalTreeStageFullPath $Root).TrimEnd('\','/')
    $pathFull = Get-UniversalTreeStageFullPath $Path
    if ($pathFull.Length -le $rootFull.Length) { return '' }
    if (-not $pathFull.StartsWith($rootFull + [System.IO.Path]::DirectorySeparatorChar,
        [System.StringComparison]::OrdinalIgnoreCase)) {
        return ''
    }
    return $pathFull.Substring($rootFull.Length + 1)
}

function Test-UniversalTreeStageWildcard {
    param([string]$Name,[string[]]$Patterns)
    foreach ($pattern in @($Patterns)) {
        if ([string]::IsNullOrWhiteSpace($pattern)) { continue }
        $wc = New-Object System.Management.Automation.WildcardPattern(
            $pattern,
            [System.Management.Automation.WildcardOptions]::IgnoreCase
        )
        if ($wc.IsMatch($Name)) { return $true }
    }
    return $false
}

function Invoke-UniversalTreeStagePortable {
    param(
        [string]$Source,
        [string]$Destination,
        [string[]]$ExcludeDirectories,
        [string[]]$ExcludeFiles
    )

    foreach ($entry in @(Get-ChildItem -LiteralPath $Source -Force -ErrorAction Stop)) {
        Copy-Item -LiteralPath $entry.FullName -Destination $Destination -Recurse -Force -ErrorAction Stop
    }

    foreach ($excluded in @($ExcludeDirectories)) {
        if ([string]::IsNullOrWhiteSpace($excluded)) { continue }
        $candidate = $excluded
        if (-not [System.IO.Path]::IsPathRooted($candidate)) {
            if ($candidate -notmatch '[\\/]') {
                foreach ($dir in @(Get-ChildItem -LiteralPath $Destination -Recurse -Directory -Force -ErrorAction SilentlyContinue |
                    Where-Object { $_.Name -ieq $candidate } |
                    Sort-Object FullName -Descending)) {
                    Remove-Item -LiteralPath $dir.FullName -Recurse -Force -ErrorAction SilentlyContinue
                }
                continue
            }
            $candidate = Join-Path $Source $candidate
        }
        $relative = Get-UniversalTreeStageRelativePath -Root $Source -Path $candidate
        if ([string]::IsNullOrWhiteSpace($relative)) { continue }
        $remove = Join-Path $Destination $relative
        if (Test-Path -LiteralPath $remove) {
            Remove-Item -LiteralPath $remove -Recurse -Force -ErrorAction SilentlyContinue
        }
    }

    if (@($ExcludeFiles).Count -gt 0) {
        foreach ($file in @(Get-ChildItem -LiteralPath $Destination -Recurse -File -Force -ErrorAction SilentlyContinue)) {
            if (Test-UniversalTreeStageWildcard -Name $file.Name -Patterns $ExcludeFiles) {
                Remove-Item -LiteralPath $file.FullName -Force -ErrorAction SilentlyContinue
            }
        }
    }
}

function Invoke-UniversalTreeStage {
    <#
      Generic ProjectOps tree staging primitive.

      Windows backend:
        robocopy /E /R:n /W:n /XJ /COPY:DAT /DCOPY:DAT

      Portable fallback:
        PowerShell filesystem copy followed by the same exclusion policy.

      Robocopy exit semantics are normalized: 0..7 = success, >=8 = failure.
      Project-specific callers provide policy; this function owns the copy engine.
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Source,
        [Parameter(Mandatory=$true)][string]$Destination,
        [string[]]$ExcludeDirectories = @(),
        [string[]]$ExcludeFiles = @(),
        [ValidateRange(0,100)][int]$RetryCount = 1,
        [ValidateRange(0,3600)][int]$RetryWaitSeconds = 1,
        [switch]$Mirror,
        [switch]$Quiet,
        [scriptblock]$OnOutput
    )

    $sourceFull = Get-UniversalTreeStageFullPath $Source
    $destinationFull = Get-UniversalTreeStageFullPath $Destination

    if (-not (Test-Path -LiteralPath $sourceFull -PathType Container)) {
        throw "TreeStage source directory does not exist: $sourceFull"
    }
    if ($sourceFull.TrimEnd('\','/') -ieq $destinationFull.TrimEnd('\','/')) {
        throw "TreeStage source and destination may not be identical."
    }

    New-Item -ItemType Directory -Force -Path $destinationFull | Out-Null

    [System.Collections.Generic.List[string]]$xd = New-Object 'System.Collections.Generic.List[string]'
    foreach ($item in @($ExcludeDirectories)) {
        if (-not [string]::IsNullOrWhiteSpace($item)) { $xd.Add([string]$item) }
    }

    $autoRelative = Get-UniversalTreeStageRelativePath -Root $sourceFull -Path $destinationFull
    if (-not [string]::IsNullOrWhiteSpace($autoRelative)) {
        $already = $false
        foreach ($item in $xd) {
            if ([string]$item -ieq $destinationFull -or [string]$item -ieq $autoRelative) { $already = $true; break }
        }
        if (-not $already) { $xd.Add($destinationFull) }
    }

    $isWindows = ($env:OS -eq 'Windows_NT')
    $robo = if ($isWindows) { Get-Command robocopy.exe -ErrorAction SilentlyContinue } else { $null }

    if ($robo) {
        [System.Collections.Generic.List[string]]$robocopyArgs = New-Object 'System.Collections.Generic.List[string]'
        $robocopyArgs.Add($sourceFull)
        $robocopyArgs.Add($destinationFull)
        $robocopyArgs.Add($(if ($Mirror) { '/MIR' } else { '/E' }))
        $robocopyArgs.Add(('/R:{0}' -f $RetryCount))
        $robocopyArgs.Add(('/W:{0}' -f $RetryWaitSeconds))
        $robocopyArgs.Add('/XJ')
        $robocopyArgs.Add('/COPY:DAT')
        $robocopyArgs.Add('/DCOPY:DAT')

        if ($xd.Count -gt 0) {
            $robocopyArgs.Add('/XD')
            foreach ($item in $xd) { $robocopyArgs.Add([string]$item) }
        }
        if (@($ExcludeFiles).Count -gt 0) {
            $robocopyArgs.Add('/XF')
            foreach ($item in @($ExcludeFiles)) {
                if (-not [string]::IsNullOrWhiteSpace($item)) { $robocopyArgs.Add([string]$item) }
            }
        }

        & $robo.Source $robocopyArgs.ToArray() 2>&1 | ForEach-Object {
            $line = [string]$_
            if ($OnOutput) { & $OnOutput $line }
            elseif (-not $Quiet) { Write-Host $line }
        }
        $code = $LASTEXITCODE
        $ok = ($code -ge 0 -and $code -le 7)
        if (-not $ok) {
            throw "Robocopy TreeStage failed with exit code $code ($sourceFull -> $destinationFull)"
        }

        return [pscustomobject]@{
            Success = $true
            Backend = 'robocopy'
            ExitCode = $code
            Source = $sourceFull
            Destination = $destinationFull
        }
    }

    if (-not $Quiet) {
        Write-Host "[WARN] Robocopy unavailable; TreeStage is using the portable filesystem backend." -ForegroundColor Yellow
    }
    Invoke-UniversalTreeStagePortable -Source $sourceFull -Destination $destinationFull `
        -ExcludeDirectories $xd.ToArray() -ExcludeFiles $ExcludeFiles

    return [pscustomobject]@{
        Success = $true
        Backend = 'portable-filesystem'
        ExitCode = 0
        Source = $sourceFull
        Destination = $destinationFull
    }
}

Export-ModuleMember -Function Invoke-UniversalTreeStage
