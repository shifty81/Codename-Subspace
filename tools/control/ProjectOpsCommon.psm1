Set-StrictMode -Version 2

# Project-neutral root-operations primitives.
# This module is intentionally free of Subspace/Havenwild gameplay assumptions.
# Project-specific policy comes from project.control.json and referenced contracts.

function Resolve-ProjectOpsRoot {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $candidate = [System.IO.Path]::GetFullPath($Root)
    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
        $candidate = Split-Path -Parent $candidate
    }

    $current = $candidate
    while (-not [string]::IsNullOrWhiteSpace($current)) {
        if (Test-Path -LiteralPath (Join-Path $current 'project.control.json') -PathType Leaf) {
            return [System.IO.Path]::GetFullPath($current)
        }
        $parent = Split-Path -Parent $current
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $current) { break }
        $current = $parent
    }

    throw ("No project.control.json found at or above: {0}" -f $Root)
}

function Get-ProjectOpsContract {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $path = Join-Path $resolved 'project.control.json'
    try {
        $contract = Get-Content -LiteralPath $path -Raw -ErrorAction Stop | ConvertFrom-Json -ErrorAction Stop
    }
    catch {
        throw ("project.control.json is unreadable: {0}: {1}" -f $path,$_.Exception.Message)
    }

    if ([string]::IsNullOrWhiteSpace([string]$contract.id)) { throw 'project.control.json is missing id.' }
    if ([string]::IsNullOrWhiteSpace([string]$contract.name)) { throw 'project.control.json is missing name.' }
    return $contract
}

function Get-ProjectOpsRelativePath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$Path
    )

    $rootFull = [System.IO.Path]::GetFullPath($Root).TrimEnd('\','/') + [System.IO.Path]::DirectorySeparatorChar
    $pathFull = [System.IO.Path]::GetFullPath($Path)
    $rootUri = New-Object System.Uri($rootFull)
    $pathUri = New-Object System.Uri($pathFull)
    return [System.Uri]::UnescapeDataString($rootUri.MakeRelativeUri($pathUri).ToString()).Replace('/','\')
}

function Get-ProjectOpsSha256Text {
    [CmdletBinding()]
    param([AllowEmptyString()][string]$Text)

    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        $bytes = [System.Text.Encoding]::UTF8.GetBytes([string]$Text)
        return ([System.BitConverter]::ToString($sha.ComputeHash($bytes))).Replace('-','').ToLowerInvariant()
    }
    finally {
        $sha.Dispose()
    }
}

function Get-ProjectOpsArtifactRoot {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $contract = Get-ProjectOpsContract -Root $resolved
    $configured = 'artifacts'
    if ($null -ne $contract.projectOps -and
        $null -ne $contract.projectOps.artifactLayout -and
        -not [string]::IsNullOrWhiteSpace([string]$contract.projectOps.artifactLayout.root)) {
        $configured = [string]$contract.projectOps.artifactLayout.root
    }

    if ([System.IO.Path]::IsPathRooted($configured)) {
        return [System.IO.Path]::GetFullPath($configured)
    }
    return [System.IO.Path]::GetFullPath((Join-Path $resolved $configured))
}

function Get-ProjectOpsArtifactPath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$Key,
        [string]$Fallback = ''
    )

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $contract = Get-ProjectOpsContract -Root $resolved
    $layout = $null
    if ($null -ne $contract.projectOps) { $layout = $contract.projectOps.artifactLayout }

    $value = ''
    if ($null -ne $layout) {
        $property = $layout.PSObject.Properties[$Key]
        if ($null -ne $property) { $value = [string]$property.Value }
    }

    if ([string]::IsNullOrWhiteSpace($value)) {
        if ([string]::IsNullOrWhiteSpace($Fallback)) {
            return Get-ProjectOpsArtifactRoot -Root $resolved
        }
        $value = $Fallback
    }

    if ([System.IO.Path]::IsPathRooted($value)) {
        return [System.IO.Path]::GetFullPath($value)
    }
    return [System.IO.Path]::GetFullPath((Join-Path $resolved $value))
}

function Resolve-ProjectOpsPython {
    [CmdletBinding()]
    param()

    $python = Get-Command python -ErrorAction SilentlyContinue
    if ($null -ne $python) {
        return [pscustomobject]@{ Path=$python.Source; Prefix=@() }
    }

    $py = Get-Command py -ErrorAction SilentlyContinue
    if ($null -ne $py) {
        return [pscustomobject]@{ Path=$py.Source; Prefix=@('-3') }
    }

    throw 'ProjectOps requires Python, but python/py was not found.'
}

function Invoke-ProjectOpsPython {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$RelativePath,
        [string[]]$Arguments = @(),
        [switch]$CaptureOutput,
        [switch]$AllowFailure
    )

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $tool = Join-Path $resolved $RelativePath
    if (-not (Test-Path -LiteralPath $tool -PathType Leaf)) {
        throw ("ProjectOps tool missing: {0}" -f $tool)
    }

    $python = Resolve-ProjectOpsPython
    $toolArgs = @($python.Prefix) + @($tool) + @($Arguments)

    $previousEap = $ErrorActionPreference
    $nativePreferenceExists = Test-Path Variable:\PSNativeCommandUseErrorActionPreference
    if ($nativePreferenceExists) { $previousNativePreference = $PSNativeCommandUseErrorActionPreference }

    try {
        $ErrorActionPreference = 'Continue'
        if ($nativePreferenceExists) { $PSNativeCommandUseErrorActionPreference = $false }

        if ($CaptureOutput) {
            $output = @(& $python.Path @toolArgs 2>&1 | ForEach-Object { [string]$_ })
        }
        else {
            & $python.Path @toolArgs
            $output = @()
        }
        $exitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousEap
        if ($nativePreferenceExists) { $PSNativeCommandUseErrorActionPreference = $previousNativePreference }
    }

    if ($exitCode -ne 0 -and -not $AllowFailure) {
        $detail = if (@($output).Count -gt 0) { ': ' + ($output -join "`n") } else { '' }
        throw ("ProjectOps tool failed with exit code {0}: {1}{2}" -f $exitCode,$RelativePath,$detail)
    }

    return [pscustomobject]@{
        ExitCode = $exitCode
        Output = @($output)
        Tool = $tool
    }
}

function Get-ProjectOpsSourceAuthoritySnapshot {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [switch]$IncludePaths
    )

    $arguments = @('--root',$Root,'snapshot','--json')
    if ($IncludePaths) { $arguments += '--include-paths' }

    $result = Invoke-ProjectOpsPython -Root $Root `
        -RelativePath 'tools\control\ProjectSourceAuthority.py' `
        -Arguments $arguments -CaptureOutput

    $text = $result.Output -join "`n"
    try {
        return ($text | ConvertFrom-Json -ErrorAction Stop)
    }
    catch {
        throw ("ProjectOps source authority returned invalid JSON: {0}" -f $text)
    }
}

function Get-ProjectOpsCertifiableSourceFingerprint {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    return [string](Get-ProjectOpsSourceAuthoritySnapshot -Root $Root).fingerprint
}

function Test-ProjectOpsGeneratedRelativePath {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$Relative
    )

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $contract = Get-ProjectOpsContract -Root $resolved
    $normalized = $Relative.Replace('/','\').TrimStart('\')
    $lower = $normalized.ToLowerInvariant()

    foreach ($fallback in @(
        '.git\','.vs\','.subspace\','updates\','artifacts\','dist\','logs\',
        'build\','out\','target\','engine\build\','engine\build-headless\','engine\out\'
    )) {
        if ($lower.StartsWith($fallback)) { return $true }
    }

    if ($lower -match '(^|\\)(__pycache__|node_modules|\.cache|\.pytest_cache)(\\|$)') { return $true }
    return $false
}

function ConvertTo-ProjectOpsProcessArgument {
    [CmdletBinding()]
    param([AllowEmptyString()][string]$Value)

    if ($null -eq $Value -or $Value.Length -eq 0) {
        return '""'
    }

    # ProcessStartInfo.Arguments uses Windows command-line parsing rules. Values
    # without whitespace/quotes can be passed through directly.
    if ($Value -notmatch '[\s"]') {
        return $Value
    }

    # Quote according to the Windows C-runtime argv rules:
    # - backslashes before a literal quote are doubled, then the quote escaped;
    # - trailing backslashes before the closing quote are doubled.
    $builder = New-Object System.Text.StringBuilder
    [void]$builder.Append([char]34)
    $backslashes = 0

    foreach ($character in $Value.ToCharArray()) {
        if ($character -eq [char]92) {
            $backslashes++
            continue
        }

        if ($character -eq [char]34) {
            if ($backslashes -gt 0) {
                [void]$builder.Append([char]92, ($backslashes * 2))
            }
            [void]$builder.Append([char]92)
            [void]$builder.Append([char]34)
            $backslashes = 0
            continue
        }

        if ($backslashes -gt 0) {
            [void]$builder.Append([char]92, $backslashes)
            $backslashes = 0
        }
        [void]$builder.Append($character)
    }

    if ($backslashes -gt 0) {
        [void]$builder.Append([char]92, ($backslashes * 2))
    }
    [void]$builder.Append([char]34)
    return $builder.ToString()
}

function Invoke-ProjectOpsGitProbe {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string[]]$GitArgs
    )

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $git = Get-Command git -ErrorAction SilentlyContinue
    if ($null -eq $git) {
        return [pscustomobject]@{
            Success = $false
            ExitCode = 9009
            StdOut = ''
            StdErr = 'git executable not found'
            Lines = @()
            ArgumentMode = 'none'
        }
    }

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $git.Source
    $psi.WorkingDirectory = $resolved
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true

    # PowerShell 7 / modern .NET exposes ProcessStartInfo.ArgumentList.
    # Windows PowerShell 5.1 / .NET Framework does not. The root launcher
    # intentionally remains compatible with stock Windows PowerShell, so use
    # ArgumentList when available and a correctly quoted Arguments string when
    # it is not.
    $argumentListProperty = $psi.PSObject.Properties['ArgumentList']
    $argumentMode = 'Arguments'
    if ($null -ne $argumentListProperty) {
        foreach ($argument in $GitArgs) {
            [void]$psi.ArgumentList.Add([string]$argument)
        }
        $argumentMode = 'ArgumentList'
    }
    else {
        $encodedArguments = @(
            $GitArgs | ForEach-Object {
                ConvertTo-ProjectOpsProcessArgument -Value ([string]$_)
            }
        )
        $psi.Arguments = ($encodedArguments -join ' ')
    }

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $psi
    try {
        if (-not $process.Start()) {
            throw 'Failed to start git process.'
        }
        $stdout = $process.StandardOutput.ReadToEnd()
        $stderr = $process.StandardError.ReadToEnd()
        $process.WaitForExit()
        $code = $process.ExitCode
    }
    finally {
        $process.Dispose()
    }

    $lines = if ([string]::IsNullOrEmpty($stdout)) {
        @()
    }
    else {
        @($stdout -split "\r?\n" | Where-Object { $_ -ne '' })
    }

    return [pscustomobject]@{
        Success = ($code -eq 0)
        ExitCode = $code
        StdOut = [string]$stdout
        StdErr = [string]$stderr
        Lines = $lines
        ArgumentMode = $argumentMode
    }
}

function Get-ProjectOpsGitRepositoryState {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $state = [ordered]@{
        initialized = $false
        hasHead = $false
        branch = ''
        head = ''
        remote = ''
    }

    if (-not (Test-Path -LiteralPath (Join-Path $resolved '.git'))) {
        return [pscustomobject]$state
    }

    if ($null -eq (Get-Command git -ErrorAction SilentlyContinue)) {
        return [pscustomobject]$state
    }

    $state.initialized = $true

    $branchProbe = Invoke-ProjectOpsGitProbe -Root $resolved -GitArgs @('branch','--show-current')
    if ($branchProbe.Success) {
        $state.branch = ([string]$branchProbe.StdOut).Trim()
    }

    $headProbe = Invoke-ProjectOpsGitProbe -Root $resolved -GitArgs @('rev-parse','--verify','HEAD')
    if ($headProbe.Success) {
        $state.head = ([string]$headProbe.StdOut).Trim()
        $state.hasHead = -not [string]::IsNullOrWhiteSpace($state.head)
    }

    $remoteProbe = Invoke-ProjectOpsGitProbe -Root $resolved -GitArgs @('remote','get-url','origin')
    if ($remoteProbe.Success) {
        $state.remote = ([string]$remoteProbe.StdOut).Trim()
    }

    return [pscustomobject]$state
}

function Get-ProjectOpsCertifiableGitStatusLines {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    if (-not (Test-Path -LiteralPath (Join-Path $resolved '.git'))) { return @() }

    $probe = Invoke-ProjectOpsGitProbe -Root $resolved -GitArgs @('status','--porcelain=v1','-uall')
    if (-not $probe.Success) {
        throw ("git status probe failed ({0}): {1}" -f $probe.ExitCode, ([string]$probe.StdErr).Trim())
    }

    return @($probe.Lines | Where-Object {
        $line = [string]$_
        $path = if ($line.Length -gt 3) { $line.Substring(3).Trim('"') } else { '' }
        if ($path -match ' -> ') { $path = ($path -split ' -> ')[-1].Trim('"') }
        -not (Test-ProjectOpsGeneratedRelativePath -Root $resolved -Relative $path)
    } | Sort-Object)
}

function Get-ProjectOpsCertifiableGitFingerprint {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $state = Get-ProjectOpsGitRepositoryState -Root $resolved
    if (-not $state.initialized -or -not $state.hasHead) {
        return $null
    }

    $status = @(Get-ProjectOpsCertifiableGitStatusLines -Root $resolved)

    $excludeArgs = @(
        '--', '.',
        ':(exclude)artifacts/**',
        ':(exclude)updates/**',
        ':(exclude).subspace/**',
        ':(exclude)dist/**',
        ':(exclude)logs/**'
    )

    $unstagedProbe = Invoke-ProjectOpsGitProbe -Root $resolved -GitArgs (@('diff','--no-ext-diff','--no-color','--binary') + $excludeArgs)
    if (-not $unstagedProbe.Success) {
        throw ("git unstaged-diff probe failed ({0}): {1}" -f $unstagedProbe.ExitCode, ([string]$unstagedProbe.StdErr).Trim())
    }

    $stagedProbe = Invoke-ProjectOpsGitProbe -Root $resolved -GitArgs (@('diff','--cached','--no-ext-diff','--no-color','--binary') + $excludeArgs)
    if (-not $stagedProbe.Success) {
        throw ("git staged-diff probe failed ({0}): {1}" -f $stagedProbe.ExitCode, ([string]$stagedProbe.StdErr).Trim())
    }

    # Git may emit advisory line-ending text such as "LF will be replaced by
    # CRLF" on stderr while still returning exit code 0. Certification is based
    # on exit status + stdout content. Advisory stderr remains diagnostic only.
    return Get-ProjectOpsSha256Text -Text (
        ([string]$state.head).Trim() + "`n" + ($status -join "`n") +
        "`n---UNSTAGED---`n" + ([string]$unstagedProbe.StdOut) +
        "`n---STAGED---`n" + ([string]$stagedProbe.StdOut)
    )
}

function Get-ProjectOpsManagedFiles {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $snapshot = Get-ProjectOpsSourceAuthoritySnapshot -Root $resolved -IncludePaths
    $result = [System.Collections.Generic.List[System.IO.FileInfo]]::new()

    foreach ($entry in @($snapshot.files)) {
        $path = Join-Path $resolved ([string]$entry.path).Replace('/','\')
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw ("Governed source file vanished while enumerating: {0}" -f $entry.path)
        }
        $result.Add((Get-Item -LiteralPath $path)) | Out-Null
    }

    return $result.ToArray()
}

function Get-ProjectOpsCommandRegistry {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $contract = Get-ProjectOpsContract -Root $Root
    if ($null -eq $contract.commands) { throw 'project.control.json is missing commands.' }

    $seen = @{}
    $commands = [System.Collections.Generic.List[object]]::new()
    foreach ($command in @($contract.commands)) {
        $key = [string]$command.key
        if ([string]::IsNullOrWhiteSpace($key)) { throw 'Project command has a blank key.' }
        $lower = $key.ToLowerInvariant()
        if ($seen.ContainsKey($lower)) { throw ("Duplicate project command key: {0}" -f $key) }
        $seen[$lower] = $true
        if ([string]::IsNullOrWhiteSpace([string]$command.label)) { throw ("Command has no label: {0}" -f $key) }
        if ([string]::IsNullOrWhiteSpace([string]$command.executable)) { throw ("Command has no executable: {0}" -f $key) }
        $commands.Add($command) | Out-Null
    }
    return @($commands.ToArray() | Sort-Object @{Expression={$_.category}}, @{Expression={$_.key}})
}

function Get-ProjectOpsCommand {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$Key
    )

    $match = @(Get-ProjectOpsCommandRegistry -Root $Root | Where-Object { [string]$_.key -ieq $Key } | Select-Object -First 1)
    if ($match.Count -eq 0) { throw ("Unknown project command: {0}" -f $Key) }
    return $match[0]
}

function Get-ProjectOpsCommandVersionLine {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Command,
        [string[]]$Arguments = @('--version')
    )

    $cmd = Get-Command $Command -ErrorAction SilentlyContinue
    if ($null -eq $cmd) {
        return [pscustomobject]@{ tool=$Command; present=$false; path=''; version='' }
    }

    $version = ''
    try {
        $version = ((& $cmd.Source @Arguments 2>&1 | Select-Object -First 1) -join '').Trim()
    }
    catch {}

    return [pscustomobject]@{ tool=$Command; present=$true; path=$cmd.Source; version=$version }
}

function Write-ProjectOpsAtomicJson {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Path,
        [Parameter(Mandatory=$true)]$Payload,
        [int]$Depth = 12
    )

    $parent = Split-Path -Parent $Path
    if (-not [string]::IsNullOrWhiteSpace($parent)) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    $tempPath = $Path + '.tmp'
    $json = ($Payload | ConvertTo-Json -Depth $Depth) + [Environment]::NewLine
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($tempPath,$json,$encoding)
    Move-Item -LiteralPath $tempPath -Destination $Path -Force
}

function Remove-ProjectOpsTransientCaches {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $roots = @(
        (Join-Path $resolved 'tools'),
        (Join-Path $resolved 'scripts')
    )

    $removed = [System.Collections.Generic.List[string]]::new()
    foreach ($scanRoot in $roots) {
        if (-not (Test-Path -LiteralPath $scanRoot -PathType Container)) { continue }
        $directories = @(Get-ChildItem -LiteralPath $scanRoot -Directory -Recurse -Force -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -in @('__pycache__','.pytest_cache','.mypy_cache') } |
            Sort-Object { $_.FullName.Length } -Descending)
        foreach ($directory in $directories) {
            if (Test-Path -LiteralPath $directory.FullName) {
                Remove-Item -LiteralPath $directory.FullName -Recurse -Force -ErrorAction SilentlyContinue
                if (-not (Test-Path -LiteralPath $directory.FullName)) {
                    $removed.Add((Get-ProjectOpsRelativePath -Root $resolved -Path $directory.FullName)) | Out-Null
                }
            }
        }
    }
    return $removed.ToArray()
}

function Invoke-ProjectOpsRootPolicyMigration {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $contract = Get-ProjectOpsContract -Root $resolved
    if ($null -eq $contract.projectOps -or $null -eq $contract.projectOps.rootPolicy) { return @() }

    $policy = $contract.projectOps.rootPolicy
    $targetRelative = if (-not [string]::IsNullOrWhiteSpace([string]$policy.legacyMigrationTarget)) {
        [string]$policy.legacyMigrationTarget
    } else {
        'artifacts\legacy\root-residue'
    }
    $targetRoot = if ([System.IO.Path]::IsPathRooted($targetRelative)) {
        $targetRelative
    } else {
        Join-Path $resolved $targetRelative
    }
    New-Item -ItemType Directory -Force -Path $targetRoot | Out-Null

    $moved = [System.Collections.Generic.List[string]]::new()
    foreach ($raw in @($policy.legacyFiles)) {
        $name = [string]$raw
        if ([string]::IsNullOrWhiteSpace($name)) { continue }
        $source = Join-Path $resolved $name
        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { continue }

        $destination = Join-Path $targetRoot $name
        $parent = Split-Path -Parent $destination
        if ($parent) { New-Item -ItemType Directory -Force -Path $parent | Out-Null }

        if (Test-Path -LiteralPath $destination -PathType Leaf) {
            $sourceHash = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
            $destinationHash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash
            if ($sourceHash -eq $destinationHash) {
                Remove-Item -LiteralPath $source -Force
                $moved.Add($name) | Out-Null
                continue
            }
            $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
            $destination = $destination + '.' + $stamp
        }

        Move-Item -LiteralPath $source -Destination $destination -Force
        $moved.Add($name) | Out-Null
    }

    return $moved.ToArray()
}

function Get-ProjectOpsSourcePolicy {
    [CmdletBinding()]
    param([Parameter(Mandatory=$true)][string]$Root)

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $contract = Get-ProjectOpsContract -Root $resolved
    $configured = 'content\architecture\projectops_source_authority_v1.json'
    if ($null -ne $contract.projectOps -and
        $null -ne $contract.projectOps.sourceAuthority -and
        -not [string]::IsNullOrWhiteSpace([string]$contract.projectOps.sourceAuthority.contract)) {
        $configured = [string]$contract.projectOps.sourceAuthority.contract
    }

    $path = if ([System.IO.Path]::IsPathRooted($configured)) {
        $configured
    } else {
        Join-Path $resolved $configured
    }
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw ("ProjectOps source-authority policy missing: {0}" -f $path)
    }

    return Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
}

function Sync-ProjectOpsStageToSourceAuthority {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$Stage
    )

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $stageFull = [System.IO.Path]::GetFullPath($Stage)
    if (-not (Test-Path -LiteralPath $stageFull -PathType Container)) {
        throw ("ProjectOps stage does not exist: {0}" -f $stageFull)
    }

    $snapshot = Get-ProjectOpsSourceAuthoritySnapshot -Root $resolved -IncludePaths
    $allowed = @{}
    foreach ($entry in @($snapshot.files)) {
        $key = ([string]$entry.path).Replace('\','/').ToLowerInvariant()
        $allowed[$key] = $entry
    }

    foreach ($file in @(Get-ChildItem -LiteralPath $stageFull -Recurse -File -Force -ErrorAction SilentlyContinue)) {
        $relative = (Get-ProjectOpsRelativePath -Root $stageFull -Path $file.FullName).Replace('\','/')
        if (-not $allowed.ContainsKey($relative.ToLowerInvariant())) {
            Remove-Item -LiteralPath $file.FullName -Force
        }
    }

    foreach ($entry in @($snapshot.files)) {
        $relative = ([string]$entry.path).Replace('/','\')
        $stagePath = Join-Path $stageFull $relative
        if (-not (Test-Path -LiteralPath $stagePath -PathType Leaf)) {
            $sourcePath = Join-Path $resolved $relative
            $parent = Split-Path -Parent $stagePath
            if ($parent) { New-Item -ItemType Directory -Force -Path $parent | Out-Null }
            Copy-Item -LiteralPath $sourcePath -Destination $stagePath -Force
        }

        $actualHash = (Get-FileHash -LiteralPath $stagePath -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -ne ([string]$entry.sha256).ToLowerInvariant()) {
            throw ("Governed stage SHA-256 mismatch: {0}" -f $entry.path)
        }
    }

    $directories = @(Get-ChildItem -LiteralPath $stageFull -Recurse -Directory -Force -ErrorAction SilentlyContinue |
        Sort-Object { $_.FullName.Length } -Descending)
    foreach ($directory in $directories) {
        if (@(Get-ChildItem -LiteralPath $directory.FullName -Force -ErrorAction SilentlyContinue).Count -eq 0) {
            Remove-Item -LiteralPath $directory.FullName -Force -ErrorAction SilentlyContinue
        }
    }

    return [pscustomobject]@{
        Fingerprint = [string]$snapshot.fingerprint
        PathCount = [int]$snapshot.pathCount
        AuthorityId = [string]$snapshot.authorityId
    }
}

function Invoke-ProjectOpsGovernedTreeStage {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$Root,
        [Parameter(Mandatory=$true)][string]$Destination,
        [scriptblock]$OnOutput,
        [switch]$Quiet
    )

    $resolved = Resolve-ProjectOpsRoot -Root $Root
    $policy = Get-ProjectOpsSourcePolicy -Root $resolved
    $treeStagePath = Join-Path $PSScriptRoot 'UniversalTreeStage.psm1'
    if (-not (Test-Path -LiteralPath $treeStagePath -PathType Leaf)) {
        throw ("Universal TreeStage module missing: {0}" -f $treeStagePath)
    }
    Import-Module $treeStagePath -Force -ErrorAction Stop

    $excludeDirectories = [System.Collections.Generic.List[string]]::new()
    foreach ($raw in @($policy.ignorePrefixes)) {
        $clean = ([string]$raw).Replace('/','\').Trim([char[]]'\/')
        if ([string]::IsNullOrWhiteSpace($clean)) { continue }
        $excludeDirectories.Add((Join-Path $resolved $clean)) | Out-Null
    }
    foreach ($raw in @($policy.ignoreDirectoryNames)) {
        $clean = [string]$raw
        if (-not [string]::IsNullOrWhiteSpace($clean)) { $excludeDirectories.Add($clean) | Out-Null }
    }

    $excludeFiles = [System.Collections.Generic.List[string]]::new()
    foreach ($raw in @($policy.ignoreExtensions)) {
        $ext = [string]$raw
        if (-not [string]::IsNullOrWhiteSpace($ext)) { $excludeFiles.Add('*' + $ext) | Out-Null }
    }
    foreach ($raw in @($policy.ignoreFileNames)) {
        $name = [string]$raw
        if (-not [string]::IsNullOrWhiteSpace($name)) { $excludeFiles.Add($name) | Out-Null }
    }
    foreach ($raw in @($policy.ignoreNamePatterns)) {
        $pattern = [string]$raw
        if (-not [string]::IsNullOrWhiteSpace($pattern)) { $excludeFiles.Add($pattern) | Out-Null }
    }

    $treeResult = Invoke-UniversalTreeStage -Source $resolved -Destination $Destination `
        -ExcludeDirectories $excludeDirectories.ToArray() `
        -ExcludeFiles $excludeFiles.ToArray() `
        -RetryCount 1 -RetryWaitSeconds 1 -Quiet:$Quiet -OnOutput $OnOutput

    $authorityResult = Sync-ProjectOpsStageToSourceAuthority -Root $resolved -Stage $Destination

    return [pscustomobject]@{
        Success = $true
        Backend = $treeResult.Backend
        ExitCode = $treeResult.ExitCode
        SourceFingerprint = $authorityResult.Fingerprint
        SourcePathCount = $authorityResult.PathCount
        SourceAuthorityId = $authorityResult.AuthorityId
    }
}

Export-ModuleMember -Function `
    Resolve-ProjectOpsRoot,Get-ProjectOpsContract,Get-ProjectOpsRelativePath,Get-ProjectOpsSha256Text,ConvertTo-ProjectOpsProcessArgument,Invoke-ProjectOpsGitProbe,`
    Get-ProjectOpsArtifactRoot,Get-ProjectOpsArtifactPath,Resolve-ProjectOpsPython,Invoke-ProjectOpsPython,`
    Get-ProjectOpsSourceAuthoritySnapshot,Get-ProjectOpsCertifiableSourceFingerprint,`
    Test-ProjectOpsGeneratedRelativePath,Get-ProjectOpsGitRepositoryState,Get-ProjectOpsCertifiableGitStatusLines,Get-ProjectOpsCertifiableGitFingerprint,`
    Get-ProjectOpsManagedFiles,Get-ProjectOpsCommandRegistry,Get-ProjectOpsCommand,Get-ProjectOpsCommandVersionLine,`
    Write-ProjectOpsAtomicJson,Remove-ProjectOpsTransientCaches,Invoke-ProjectOpsRootPolicyMigration,`
    Get-ProjectOpsSourcePolicy,Sync-ProjectOpsStageToSourceAuthority,Invoke-ProjectOpsGovernedTreeStage
