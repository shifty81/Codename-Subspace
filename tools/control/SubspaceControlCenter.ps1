param(
    [string]$Root = (Get-Location).Path,
    [ValidateSet(
        'health','self-test','environment','audit-package','capture-baseline',
        'incremental-handoff','warnings','git-status','git-init','git-history',
        'git-commit-green','git-push','git-remote',
        'repo-authority-audit','repo-authority-prepare','repo-authority-publish','git-repair'
    )]
    [string]$Action = 'health',
    [string]$PublishConfirmation = ''
)
$ErrorActionPreference = 'Stop'
$Root = [System.IO.Path]::GetFullPath($Root)
. (Join-Path $PSScriptRoot 'ControlCenterCommon.ps1')

$state = Join-Path $Root '.subspace'
$ccState = Join-Path $state 'control-center'
$artifactRoot = Join-Path $Root 'artifacts'
$packaging = Join-Path $artifactRoot 'baselines'
$auditRoot = Join-Path $artifactRoot 'audits'
$patchRoot = Join-Path $artifactRoot 'patches'
$tempRoot = Join-Path $artifactRoot 'temp'
$logsRoot = Join-Path $artifactRoot 'logs'
$debugRoot = Join-Path $artifactRoot 'debug'
foreach ($p in @($state,$ccState,$artifactRoot,$packaging,$auditRoot,$patchRoot,$tempRoot,$logsRoot,$debugRoot)) { New-Item -ItemType Directory -Force -Path $p | Out-Null }

function Write-Section([string]$Text) {
    Write-Host ''
    Write-Host $Text -ForegroundColor Cyan
    Write-Host ('-' * [Math]::Min(72,[Math]::Max(8,$Text.Length)))
}

function Get-HealthRecord {
    $gatePath = Join-Path $state 'last-green-quality-gate.json'
    $gate = $null
    if (Test-Path -LiteralPath $gatePath) { try { $gate = Get-Content -LiteralPath $gatePath -Raw | ConvertFrom-Json } catch {} }
    $updateLatest = Join-Path $Root 'updates\LATEST_UPDATE_APPLY.txt'
    $updateState = if (Test-Path -LiteralPath $updateLatest) {
        $line = Get-Content -LiteralPath $updateLatest | Where-Object { $_ -match '^Result:' } | Select-Object -First 1
        if ($line) { ($line -replace '^Result:\s*','').Trim() } else { 'UNKNOWN' }
    } else { 'NONE' }
    $baseline = Test-Path -LiteralPath (Join-Path $packaging 'baseline.json')
    $gitState = 'Not initialized'
    if (Test-Path -LiteralPath (Join-Path $Root '.git')) {
        $lines = @(Get-CertifiableGitStatusLines -Root $Root)
        $gitState = if ($lines.Count -eq 0) { 'Clean' } else { 'Modified' }
    }
    $checks = [ordered]@{
        rootLauncher = Test-Path -LiteralPath (Join-Path $Root 'SubspaceTools.cmd')
        rootControl = Test-Path -LiteralPath (Join-Path $Root 'SubspaceTools.ps1')
        universalContract = Test-Path -LiteralPath (Join-Path $Root 'project.control.json')
        nativeCmake = Test-Path -LiteralPath (Join-Path $Root 'engine\CMakeLists.txt')
        nativeMain = Test-Path -LiteralPath (Join-Path $Root 'engine\src\main.cpp')
        updater = Test-Path -LiteralPath (Join-Path $Root 'scripts\subspace_apply_update_inbox.ps1')
        supplyChain = Test-Path -LiteralPath (Join-Path $Root 'scripts\subspace_supply_chain_gate.ps1')
        debugBundle = Test-Path -LiteralPath (Join-Path $debugRoot 'LATEST_DEBUG_BUNDLE.txt')
        passContinuityAudit = Test-Path -LiteralPath (Join-Path $Root 'scripts\subspace_pass_continuity_audit.ps1')
        repositoryAuthority = Test-Path -LiteralPath (Join-Path $Root 'tools\control\NormalizeGitHubAuthority.ps1')
        projectOpsCli = Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsCli.py')
        commandRegistry = Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectCommandRegistry.ps1')
        sourceAuthority = Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectSourceAuthority.py')
        projectShell = Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectShell.py')
        gitRepair = Test-Path -LiteralPath (Join-Path $Root 'tools\control\RepairGitWorkingCopy.py')
        projectOpsCommon = Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsCommon.psm1')
        projectOpsRootAudit = Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsRootAudit.ps1')
        projectOpsMaintenance = Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsMaintenance.ps1')
    }
    $passed = @($checks.GetEnumerator() | Where-Object Value).Count
    return [pscustomobject]@{
        timestamp=(Get-Date).ToString('o')
        project='Codename Subspace'
        root=$Root
        score=[int][Math]::Round(($passed / [double]$checks.Count) * 100)
        checks=$checks
        gate=if ($gate) { "$($gate.result) $($gate.gateId)" } else { 'None' }
        git=$gitState
        baseline=if ($baseline) { 'Ready' } else { 'Missing' }
        update=$updateState
        runtimeAuthority='native C++ only'
        legacyCSharp='inert reference archive'
    }
}

function Invoke-Health {
    $h = Get-HealthRecord
    Write-Host '========================================================================'
    Write-Host ' CODENAME SUBSPACE PROJECT HEALTH'
    Write-Host '========================================================================'
    Write-Host (" Score       : {0}/100" -f $h.score)
    Write-Host (" Gate        : {0}" -f $h.gate)
    Write-Host (" Git         : {0}" -f $h.git)
    Write-Host (" Baseline    : {0}" -f $h.baseline)
    Write-Host (" Update      : {0}" -f $h.update)
    Write-Host (" Runtime     : {0}" -f $h.runtimeAuthority)
    Write-Host '------------------------------------------------------------------------'
    foreach ($entry in $h.checks.GetEnumerator()) {
        Write-Host (" [{0}] {1}" -f $(if ($entry.Value) {'PASS'} else {'FAIL'}),$entry.Key) -ForegroundColor $(if ($entry.Value) {'Green'} else {'Red'})
    }
    $path = Join-Path $ccState 'health-latest.json'
    $h | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $path -Encoding UTF8
    Write-Host "Health record: $path"
}

function Invoke-SelfTest {
    $tests = [System.Collections.Generic.List[object]]::new()
    function T([string]$name,[bool]$ok,[string]$detail='') { $tests.Add([pscustomobject]@{name=$name;pass=$ok;detail=$detail}) | Out-Null }
    T 'root launcher' (Test-Path -LiteralPath (Join-Path $Root 'SubspaceTools.cmd'))
    T 'root PowerShell control' (Test-Path -LiteralPath (Join-Path $Root 'SubspaceTools.ps1'))
    T 'project.control.json' (Test-Path -LiteralPath (Join-Path $Root 'project.control.json'))
    try { Get-Content -LiteralPath (Join-Path $Root 'project.control.json') -Raw | ConvertFrom-Json | Out-Null; T 'project.control.json parses' $true } catch { T 'project.control.json parses' $false $_.Exception.Message }
    $updater = Join-Path $Root 'scripts\subspace_apply_update_inbox.ps1'
    $u = if (Test-Path -LiteralPath $updater) { Get-Content -LiteralPath $updater -Raw } else { '' }
    T 'updater exists' ([bool]$u)
    T 'pass/source continuity audit exists' (Test-Path -LiteralPath (Join-Path $Root 'scripts\subspace_pass_continuity_audit.ps1'))
    T 'Pass655-674 guarded recovery exists' (Test-Path -LiteralPath (Join-Path $Root 'scripts\subspace_recover_pass655_674.ps1'))
    T 'updater generic-list constructor hardened' ($u -notmatch 'New-Object\s+[^\r\n]*Generic\.List')
    T 'manifest list serialization hardened' ($u.Contains('$fileEntries.ToArray()') -and $u.Contains('$removeEntries.ToArray()'))
    T 'transaction serialization hardened' ($u.Contains('$transactionEntries.ToArray()'))
    $rootToolsPath = Join-Path $Root 'SubspaceTools.ps1'
    $rootTools = if (Test-Path -LiteralPath $rootToolsPath) { Get-Content -LiteralPath $rootToolsPath -Raw } else { '' }
    $launcherPath = Join-Path $Root 'SubspaceTools.cmd'
    $launcher = if (Test-Path -LiteralPath $launcherPath) { Get-Content -LiteralPath $launcherPath -Raw } else { '' }
    $undoPath = Join-Path $Root 'tools\control\InvokePatchUndo.ps1'
    $undo = if (Test-Path -LiteralPath $undoPath) { Get-Content -LiteralPath $undoPath -Raw } else { '' }
    T 'interactive final PASS/FAIL banner present' ($rootTools.Contains('Show-GateCertificationBanner'))
    T 'interactive gates do not exit from gate functions' (-not $rootTools.Contains('if ($failures -gt 0) { exit 1 }'))
    T 'failed launcher remains visible' ($launcher.Contains('PROCESS EXITED WITH FAILURE') -and $launcher.Contains('pause'))
    T 'undo guards recreated removed paths' ($undo.Contains('removed by the patch has since been recreated'))
    T 'undo guards modified overlay paths' ($undo.Contains('written by the patch has since been modified'))
    T 'control module root binding uses external runner' ($rootTools.Contains('Invoke-ProjectScript -RelativePath $RelativePath -Arguments $Arguments'))
    T 'root launcher is immutable during normal patch intake' ($u.Contains('SubspaceTools.cmd is the active root launcher and is bootstrap-only'))
    T 'self-update child preserves interactive parent context' ($rootTools.Contains('SUBSPACE_INTERACTIVE_PARENT'))
    T 'console palette recovery present' ($rootTools.Contains('Reset-SubspaceConsolePalette') -and $rootTools.Contains('Write-ChildOutputLine'))
    T 'child ANSI reset stripping present' ($rootTools.Contains('Remove-SubspaceAnsiSequences'))
    T 'stale -Root binding residue auto-repair present' ($rootTools.Contains('Repair-StaleRootBindingResidue'))
    T 'source rollup excludes operational artifacts' ($rootTools.Contains('(Join-Path $Global:SubspaceRoot "artifacts")') -and $rootTools.Contains('(Join-Path $Global:SubspaceRoot "-Root")'))
    T 'universal ProjectOps TreeStage module present' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\UniversalTreeStage.psm1'))
    T 'root utility consumes ProjectOps TreeStage' ($rootTools.Contains('Invoke-UniversalTreeStage') -and $rootTools.Contains('Invoke-ProjectTreeStage'))
    $repoNormalizePath = Join-Path $Root 'tools\control\NormalizeGitHubAuthority.ps1'
    $repoNormalizeText = if(Test-Path -LiteralPath $repoNormalizePath){Get-Content -LiteralPath $repoNormalizePath -Raw}else{''}
    T 'repository normalization consumes ProjectOps TreeStage' ($repoNormalizeText.Contains('Invoke-UniversalTreeStage'))
    T 'repository normalization Git argument binding repaired' (-not $repoNormalizeText.Contains(' -Args @('))
    T 'canonical artifact root owns debug output' ($rootTools.Contains('Join-Path $Global:ArtifactRoot "debug"'))
    T 'canonical artifact root owns source output' ($rootTools.Contains('Join-Path $Global:ArtifactRoot "source"'))
    T 'failure handoff opens current debug ZIP' ($rootTools.Contains('Open-DebugHandoffFolder') -and $rootTools.Contains('/select,'))
    $repoAuthorityPath = Join-Path $Root 'tools\control\NormalizeGitHubAuthority.ps1'
    $repoAuthority = if (Test-Path -LiteralPath $repoAuthorityPath) { Get-Content -LiteralPath $repoAuthorityPath -Raw } else { '' }
    T 'repository authority tool present' (Test-Path -LiteralPath $repoAuthorityPath)
    T 'repository publish archives previous main' ($repoAuthority.Contains('archive/pre-native-normalization-'))
    T 'repository publish uses force-with-lease' ($repoAuthority.Contains('--force-with-lease=refs/heads/main:'))
    T 'repository publish requires GREEN fingerprint' ($repoAuthority.Contains('Get-CertifiableGitFingerprint') -and $repoAuthority.Contains('Type PUBLISH'))
    T 'ProjectOps command registry exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectCommandRegistry.ps1'))
    T 'ProjectOps machine CLI exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsCli.py'))
    T 'ProjectOps source authority exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectSourceAuthority.py'))
    T 'ProjectOps logged shell exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectShell.py'))
    T 'ProjectOps safe Git repair exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\RepairGitWorkingCopy.py'))
    T 'ProjectOps common root-tooling module exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsCommon.psm1'))
    T 'ProjectOps root audit exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsRootAudit.ps1'))
    T 'ProjectOps maintenance/repair exists' (Test-Path -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsMaintenance.ps1'))
    T 'Subspace common layer is compatibility-only' ((Get-Content -LiteralPath (Join-Path $Root 'tools\control\ControlCenterCommon.ps1') -Raw).Contains('Compatibility adapter for existing Subspace control scripts'))
    T 'root utility avoids reserved automatic args assignment' (-not ($rootTools -match '(?m)^\s*\$args\s*='))
    T 'root utility avoids invalid LASTEXITCODE colon interpolation' (-not $rootTools.Contains(('$LASTEXITCODE' + ':')))
    T 'source rollup consumes governed source authority' ($rootTools.Contains('Invoke-ProjectOpsGovernedTreeStage'))
    try { $sourceSnapshot = Get-ProjectSourceAuthoritySnapshot -Root $Root; T 'ProjectOps source authority snapshot valid' (-not [string]::IsNullOrWhiteSpace([string]$sourceSnapshot.fingerprint)) } catch { T 'ProjectOps source authority snapshot valid' $false $_.Exception.Message }
    $rootAudit = Join-Path $Root 'scripts\subspace_root_cleanliness_audit.ps1'
    $ra = if (Test-Path -LiteralPath $rootAudit) { Get-Content -LiteralPath $rootAudit -Raw } else { '' }
    T 'root audit delegates to ProjectOps root authority' ($ra.Contains('ProjectOpsRootAudit.ps1'))
    foreach ($name in @('WriteQualityGateRecord.ps1','BuildArtifactIndex.ps1','CompareQualityGates.ps1','GetDependencyStatus.ps1','InvokePatchUndo.ps1','SubspaceControlCenter.ps1')) {
        T ("control module: " + $name) (Test-Path -LiteralPath (Join-Path $PSScriptRoot $name))
    }
    $failed = @($tests | Where-Object { -not $_.pass })
    Write-Host '========================================================================'
    Write-Host ' SUBSPACE CONTROL CENTER SELF-TEST'
    Write-Host '========================================================================'
    foreach ($t in $tests) {
        Write-Host ("[{0}] {1}" -f $(if ($t.pass) {'PASS'} else {'FAIL'}),$t.name) -ForegroundColor $(if ($t.pass) {'Green'} else {'Red'})
        if ($t.detail) { Write-Host ("       " + $t.detail) -ForegroundColor DarkGray }
    }
    $record = [pscustomobject]@{timestamp=(Get-Date).ToString('o');passed=$tests.Count-$failed.Count;failed=$failed.Count;tests=$tests.ToArray()}
    $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $ccState 'self-test-latest.json') -Encoding UTF8
    if ($failed.Count -gt 0) { throw "Control Center self-test failed: $($failed.Count) check(s)." }
}

function Get-EnvironmentRecords {
    $rows = [System.Collections.Generic.List[object]]::new()
    foreach ($spec in @(
        @('pwsh',@('--version')), @('powershell',@('-NoProfile','-Command','$PSVersionTable.PSVersion.ToString()')),
        @('git',@('--version')), @('cmake',@('--version')), @('ctest',@('--version')),
        @('python',@('--version')), @('py',@('--version')), @('blender',@('--version')),
        @('msbuild',@('-version','-nologo')), @('cl',@())
    )) {
        $rows.Add((Get-CommandVersionLine -Command $spec[0] -Arguments $spec[1])) | Out-Null
    }
    return $rows.ToArray()
}

function Invoke-Environment {
    $rows = @(Get-EnvironmentRecords)
    Write-Host '========================================================================'
    Write-Host ' SUBSPACE ENVIRONMENT / TOOLCHAIN INVENTORY'
    Write-Host '========================================================================'
    foreach ($r in $rows) {
        Write-Host ("[{0}] {1,-12} {2}" -f $(if ($r.present) {'PASS'} else {'MISS'}),$r.tool,$r.version) -ForegroundColor $(if ($r.present) {'Green'} else {'Yellow'})
        if ($r.path) { Write-Host ("       " + $r.path) -ForegroundColor DarkGray }
    }
    $record = [pscustomobject]@{timestamp=(Get-Date).ToString('o');root=$Root;tools=$rows}
    $path = Join-Path $ccState 'environment-latest.json'
    $record | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $path -Encoding UTF8
    Write-Host "Environment record: $path"
}

function Get-BaselineData {
    $path = Join-Path $packaging 'baseline.json'
    if (-not (Test-Path -LiteralPath $path)) { return $null }
    return Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
}

function Invoke-CaptureBaseline {
    $entries = [System.Collections.Generic.List[object]]::new()
    foreach ($file in @(Get-ManagedProjectFiles -Root $Root)) {
        $entries.Add([pscustomobject]@{
            path=(Get-SubspaceRelativePath -Root $Root -Path $file.FullName)
            bytes=[int64]$file.Length
            sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $file.FullName).Hash.ToLowerInvariant()
        }) | Out-Null
    }
    $record = [pscustomobject]@{
        schemaVersion=1
        project='Codename Subspace'
        timestamp=(Get-Date).ToString('o')
        root=$Root
        files=$entries.ToArray()
    }
    $path = Join-Path $packaging 'baseline.json'
    $record | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $path -Encoding UTF8
    Write-Host ("[PASS] Packaging baseline captured: {0} files" -f $entries.Count) -ForegroundColor Green
    Write-Host $path
}

function Invoke-IncrementalHandoff {
    $baseline = Get-BaselineData
    if (-not $baseline) { throw 'No packaging baseline exists. Capture a baseline first.' }
    $before = @{}
    foreach ($e in @($baseline.files)) { $before[[string]$e.path.ToLowerInvariant()] = $e }
    $current = @{}
    $changed = [System.Collections.Generic.List[object]]::new()
    foreach ($file in @(Get-ManagedProjectFiles -Root $Root)) {
        $rel = Get-SubspaceRelativePath -Root $Root -Path $file.FullName
        $sha = (Get-FileHash -Algorithm SHA256 -LiteralPath $file.FullName).Hash.ToLowerInvariant()
        $current[$rel.ToLowerInvariant()] = [pscustomobject]@{path=$rel;bytes=[int64]$file.Length;sha256=$sha;full=$file.FullName}
        if (-not $before.ContainsKey($rel.ToLowerInvariant()) -or ([string]$before[$rel.ToLowerInvariant()].sha256).ToLowerInvariant() -ne $sha) {
            $changed.Add($current[$rel.ToLowerInvariant()]) | Out-Null
        }
    }
    $removed = [System.Collections.Generic.List[string]]::new()
    foreach ($key in $before.Keys) {
        if (-not $current.ContainsKey($key)) { $removed.Add([string]$before[$key].path) | Out-Null }
    }
    if ($changed.Count -eq 0 -and $removed.Count -eq 0) {
        Write-Host '[PASS] No source changes since the packaging baseline.' -ForegroundColor Green
        return
    }
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $stage = Join-Path $tempRoot ("handoff-" + $stamp)
    if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $stage | Out-Null
    $manifestEntries = [System.Collections.Generic.List[object]]::new()
    foreach ($e in $changed) {
        $dest = Join-Path $stage $e.path
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
        Copy-Item -LiteralPath $e.full -Destination $dest -Force
        $manifestEntries.Add([pscustomobject]@{path=$e.path;bytes=$e.bytes;sha256=$e.sha256}) | Out-Null
    }
    $manifest = [pscustomobject]@{
        schemaVersion=1
        patchId=("Pass745-GeneratedIncremental-" + $stamp)
        project='Codename Subspace'
        createdAt=(Get-Date).ToString('o')
        files=$manifestEntries.ToArray()
        remove=$removed.ToArray()
    }
    $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $stage 'PATCH_MANIFEST.json') -Encoding UTF8
    New-Item -ItemType Directory -Force -Path $patchRoot | Out-Null
    $zip = Join-Path $patchRoot ("Codename_Subspace_Patch_Generated_" + $stamp + ".zip")
    if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force }
    Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $zip -CompressionLevel Optimal
    Remove-Item -LiteralPath $stage -Recurse -Force
    Write-Host ("[PASS] Incremental handoff: {0} changed, {1} removed" -f $changed.Count,$removed.Count) -ForegroundColor Green
    Write-Host $zip
}

function Invoke-Warnings {
    $buildRoot = Join-Path $logsRoot 'builds'
    $log = Get-ChildItem -LiteralPath $buildRoot -File -ErrorAction SilentlyContinue | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
    if (-not $log) { Write-Host 'No build logs are available.'; return }
    $lines = @(Get-Content -LiteralPath $log.FullName -ErrorAction SilentlyContinue | Where-Object { $_ -match '(?i)\bwarning\b|\berror\b' })
    Write-Host "Latest build log: $($log.FullName)"
    Write-Host ("Warnings/errors matched: {0}" -f $lines.Count)
    $lines | Select-Object -First 100 | ForEach-Object { Write-Host $_ }
    if ($lines.Count -gt 100) { Write-Host ("... {0} additional lines omitted" -f ($lines.Count-100)) }
}

function Invoke-AuditPackage {
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $audit = Join-Path $auditRoot ("AUD-" + $stamp)
    $reports = Join-Path $audit 'reports'
    $sourceClean = Join-Path $audit 'source-clean'
    New-Item -ItemType Directory -Force -Path $reports,$sourceClean | Out-Null

    $all = @(Get-ChildItem -LiteralPath $Root -Recurse -File -Force -ErrorAction SilentlyContinue)
    $managed = @(Get-ManagedProjectFiles -Root $Root)
    $managedMap = @{}
    foreach ($f in $managed) { $managedMap[(Get-SubspaceRelativePath -Root $Root -Path $f.FullName).ToLowerInvariant()] = $true }

    $manifest = [System.Collections.Generic.List[object]]::new()
    $excluded = [System.Collections.Generic.List[object]]::new()
    $large = [System.Collections.Generic.List[object]]::new()
    $hashGroups = @{}
    foreach ($f in $all) {
        $rel = Get-SubspaceRelativePath -Root $Root -Path $f.FullName
        if ($managedMap.ContainsKey($rel.ToLowerInvariant())) {
            $sha = (Get-FileHash -Algorithm SHA256 -LiteralPath $f.FullName).Hash.ToLowerInvariant()
            $row = [pscustomobject]@{path=$rel;bytes=[int64]$f.Length;sha256=$sha}
            $manifest.Add($row) | Out-Null
            if ($f.Length -ge 25MB) { $large.Add($row) | Out-Null }
            if (-not $hashGroups.ContainsKey($sha)) { $hashGroups[$sha] = [System.Collections.Generic.List[string]]::new() }
            $hashGroups[$sha].Add($rel) | Out-Null
            if ($f.Length -le 16MB) {
                $dest = Join-Path $sourceClean $rel
                New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
                Copy-Item -LiteralPath $f.FullName -Destination $dest -Force
            }
        } else {
            $excluded.Add([pscustomobject]@{path=$rel;bytes=[int64]$f.Length;reason='generated/archive/excluded'}) | Out-Null
        }
    }
    $manifest.ToArray() | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $reports 'file-manifest.csv')
    $excluded.ToArray() | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $reports 'excluded-files.csv')
    $large.ToArray() | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $reports 'large-files.csv')
    $dupes = [System.Collections.Generic.List[object]]::new()
    foreach ($sha in $hashGroups.Keys) {
        if ($hashGroups[$sha].Count -gt 1) {
            $dupes.Add([pscustomobject]@{sha256=$sha;count=$hashGroups[$sha].Count;paths=($hashGroups[$sha].ToArray() -join ' | ')}) | Out-Null
        }
    }
    $dupes.ToArray() | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $reports 'duplicate-hashes.csv')
    $manifest | Sort-Object path | ForEach-Object { $_.path } | Set-Content -LiteralPath (Join-Path $reports 'tree.txt') -Encoding UTF8

    $jsonRows = [System.Collections.Generic.List[object]]::new()
    foreach ($f in $managed | Where-Object Extension -eq '.json') {
        try { Get-Content -LiteralPath $f.FullName -Raw | ConvertFrom-Json | Out-Null; $ok=$true; $err='' } catch { $ok=$false; $err=$_.Exception.Message }
        $jsonRows.Add([pscustomobject]@{path=(Get-SubspaceRelativePath -Root $Root -Path $f.FullName);valid=$ok;error=$err}) | Out-Null
    }
    $jsonRows.ToArray() | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $reports 'json-validation.csv')
    @(Get-EnvironmentRecords) | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $reports 'environment.csv')

    $health = Get-HealthRecord
    $health | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $reports 'project-health.json') -Encoding UTF8

    $gitStatus = @()
    $gitState = Get-ProjectGitRepositoryState -Root $Root
    $gitSummary = [ordered]@{
        initialized=[bool]$gitState.initialized
        hasHead=[bool]$gitState.hasHead
        branch=[string]$gitState.branch
        head=[string]$gitState.head
        remote=[string]$gitState.remote
    }
    if ($gitState.initialized) {
        $gitStatus = @(Get-CertifiableGitStatusLines -Root $Root)
    }
    $gitStatus | Set-Content -LiteralPath (Join-Path $reports 'git-status.txt') -Encoding UTF8
    $gitSummary | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $reports 'git-summary.json') -Encoding UTF8

    $ccInventory = @(Get-ChildItem -LiteralPath $Root -Recurse -File -ErrorAction SilentlyContinue | Where-Object {
        $_.Name -match '(?i)controlcenter|projectcontrol|subspacetools|rootutility'
    } | ForEach-Object {
        [pscustomobject]@{path=(Get-SubspaceRelativePath -Root $Root -Path $_.FullName);bytes=[int64]$_.Length}
    })
    $ccInventory | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $reports 'control-center-inventory.csv')

    $baseline = Get-BaselineData
    $comparison = [ordered]@{baselinePresent=[bool]$baseline;baselineTimestamp=$(if($baseline){$baseline.timestamp}else{$null});managedFiles=$managed.Count}
    $comparison | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $reports 'baseline-comparison.json') -Encoding UTF8

    $normalization = [ordered]@{
        rootLauncher='SubspaceTools.cmd'
        rootAuthority='SubspaceTools.ps1'
        universalContract='project.control.json'
        projectAdapter='tools/control/SubspaceControlCenter.ps1'
        runtimeAuthority='native C++ only'
        competingControlCenters=@($ccInventory | Where-Object { $_.path -notmatch '(?i)^SubspaceTools\.(cmd|ps1)$|^tools\\control\\' } | Select-Object -ExpandProperty path)
    }
    $normalization | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $reports 'normalization-audit.json') -Encoding UTF8

    $summary = @(
        '# Codename Subspace Project Audit',
        '',
        "Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss zzz')",
        "Root: $Root",
        '',
        "Managed source files: $($managed.Count)",
        "Excluded/generated files: $($excluded.Count)",
        "Large managed files (>=25 MB): $($large.Count)",
        "Duplicate hash groups: $($dupes.Count)",
        "JSON files checked: $($jsonRows.Count)",
        "Health score: $($health.score)/100",
        '',
        'The source-clean directory intentionally excludes managed files above 16 MB; those remain represented by hash in reports.'
    )
    $summary | Set-Content -LiteralPath (Join-Path $audit 'PROJECT_AUDIT_SUMMARY.md') -Encoding UTF8
    [pscustomobject]@{schemaVersion=1;timestamp=$stamp;root=$Root;health=$health.score;managedFiles=$managed.Count} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $audit 'AUDIT_MANIFEST.json') -Encoding UTF8

    $zipDir = Join-Path $Root 'artifacts\audit-packages'
    New-Item -ItemType Directory -Force -Path $zipDir | Out-Null
    $zip = Join-Path $zipDir ("Codename_Subspace_AuditPackage_" + $stamp + ".zip")
    Compress-Archive -Path (Join-Path $audit '*') -DestinationPath $zip -CompressionLevel Optimal
    Write-Host "[PASS] Audit package created:" -ForegroundColor Green
    Write-Host $zip
}

function Require-Git {
    $git = Get-Command git -ErrorAction SilentlyContinue
    if (-not $git) { throw 'Git is not installed or not on PATH.' }
    return $git.Source
}

function Invoke-GitStatus {
    $git=Require-Git
    if (-not (Test-Path -LiteralPath (Join-Path $Root '.git'))) { Write-Host 'Git is not initialized for this project.'; return }
    Push-Location $Root
    try {
        & $git status
        Write-Host ''
        Write-Host 'Remotes:'
        & $git remote -v
    } finally { Pop-Location }
}

function Invoke-GitInit {
    $git=Require-Git
    if (Test-Path -LiteralPath (Join-Path $Root '.git')) { Write-Host 'Git is already initialized.'; return }
    Push-Location $Root
    try {
        & $git init -b main
        if ($LASTEXITCODE -ne 0) { & $git init; & $git branch -M main }
    } finally { Pop-Location }
    Write-Host '[PASS] Git repository initialized.' -ForegroundColor Green
}

function Invoke-GitHistory {
    $git=Require-Git
    if (-not (Test-Path -LiteralPath (Join-Path $Root '.git'))) { throw 'Git is not initialized.' }
    Push-Location $Root
    try { & $git log --oneline --decorate --graph -20 } finally { Pop-Location }
}

function Invoke-GitCommitGreen {
    $git=Require-Git
    if (-not (Test-Path -LiteralPath (Join-Path $Root '.git'))) { throw 'Git is not initialized.' }
    $greenPath = Join-Path $state 'last-green-quality-gate.json'
    if (-not (Test-Path -LiteralPath $greenPath)) { throw 'No GREEN quality-gate marker exists.' }
    $green = Get-Content -LiteralPath $greenPath -Raw | ConvertFrom-Json
    if ([string]$green.result -ne 'PASS') { throw 'Latest promotion marker is not PASS.' }
    if (-not $green.gitFingerprint) { throw 'GREEN marker predates standardized Git fingerprinting. Run Full quality gate once first.' }
    if (-not $green.sourceFingerprint) { throw 'GREEN marker predates ProjectOps source authority. Run Full Quality Gate once first.' }
    $current = Get-CertifiableGitFingerprint -Root $Root
    if ($current -ne [string]$green.gitFingerprint) { throw 'Certified Git state has changed since the GREEN gate. Run Full quality gate again before committing.' }
    $sourceSnapshot = Get-ProjectSourceAuthoritySnapshot -Root $Root
    if ([string]$sourceSnapshot.fingerprint -ne [string]$green.sourceFingerprint -or [int]$sourceSnapshot.pathCount -ne [int]$green.sourcePathCount) {
        throw 'Governed source bytes/path set changed since the GREEN gate. Run Full Quality Gate again before committing.'
    }
    $message = Read-Host 'Commit message (blank uses certified gate id)'
    if ([string]::IsNullOrWhiteSpace($message)) { $message = "Certified $($green.gateId)" }
    Push-Location $Root
    try {
        & $git add -A -- . ':(exclude)logs/**' ':(exclude)dist/**' ':(exclude)updates/**' ':(exclude)artifacts/**' ':(exclude).subspace/**' ':(exclude)engine/build/**' ':(exclude)engine/build-headless/**'
        if ($LASTEXITCODE -ne 0) { throw 'git add failed.' }
        & $git commit -m $message
        if ($LASTEXITCODE -ne 0) { throw 'git commit failed.' }
    } finally { Pop-Location }
    Write-Host '[PASS] Certified GREEN source committed.' -ForegroundColor Green
}

function Invoke-GitPush {
    $git=Require-Git
    if (-not (Test-Path -LiteralPath (Join-Path $Root '.git'))) { throw 'Git is not initialized.' }
    Push-Location $Root
    try {
        $branch = [string](& $git branch --show-current)
        if ([string]::IsNullOrWhiteSpace($branch)) { throw 'Current branch could not be determined.' }
        & $git push -u origin $branch
        if ($LASTEXITCODE -ne 0) { throw 'git push failed.' }
    } finally { Pop-Location }
}

function Invoke-GitRemote {
    $git=Require-Git
    if (-not (Test-Path -LiteralPath (Join-Path $Root '.git'))) { throw 'Git is not initialized.' }
    Push-Location $Root
    try {
        Write-Host 'Current remotes:'
        & $git remote -v
        $existing = [string](& $git remote get-url origin 2>$null)
        if ([string]::IsNullOrWhiteSpace($existing)) {
            $url = Read-Host 'No origin is configured. Enter GitHub/repository URL to add, or leave blank'
            if (-not [string]::IsNullOrWhiteSpace($url)) {
                & $git remote add origin $url
                if ($LASTEXITCODE -ne 0) { throw 'Failed to add origin.' }
                Write-Host '[PASS] origin configured.' -ForegroundColor Green
            }
        }
    } finally { Pop-Location }
}


function Invoke-GitRepairWorkingCopy {
    $tool = Join-Path $Root 'tools\control\RepairGitWorkingCopy.py'
    if(-not (Test-Path -LiteralPath $tool -PathType Leaf)){throw "ProjectOps Git repair tool missing: $tool"}
    $python = Resolve-ProjectOpsPython
    $repairArgs = @($python.Prefix) + @($tool,'--root',$Root)
    & $python.Path @repairArgs
    if($LASTEXITCODE -ne 0){throw "ProjectOps Git repair failed with exit code $LASTEXITCODE."}
}

function Invoke-RepositoryAuthority {
    param(
        [ValidateSet('Audit','Prepare','Publish')][string]$Mode,
        [string]$Confirmation = ''
    )
    $script=Join-Path $Root 'tools\control\NormalizeGitHubAuthority.ps1'
    if(-not (Test-Path -LiteralPath $script)){throw "Repository authority tool missing: $script"}

    $authorityArgs=@(
        '-NoProfile','-ExecutionPolicy','Bypass',
        '-File',$script,
        '-Root',$Root,
        '-Mode',$Mode
    )
    if($Mode -eq 'Publish'){
        $authorityArgs += @('-PublishConfirmation',$Confirmation)
    }

    & powershell.exe @authorityArgs
    if($LASTEXITCODE -ne 0){throw "Repository authority $Mode failed with exit code $LASTEXITCODE."}
}

switch ($Action) {
    'health' { Invoke-Health }
    'self-test' { Invoke-SelfTest }
    'environment' { Invoke-Environment }
    'audit-package' { Invoke-AuditPackage }
    'capture-baseline' { Invoke-CaptureBaseline }
    'incremental-handoff' { Invoke-IncrementalHandoff }
    'warnings' { Invoke-Warnings }
    'git-status' { Invoke-GitStatus }
    'git-init' { Invoke-GitInit }
    'git-history' { Invoke-GitHistory }
    'git-commit-green' { Invoke-GitCommitGreen }
    'git-push' { Invoke-GitPush }
    'git-remote' { Invoke-GitRemote }
    'git-repair' { Invoke-GitRepairWorkingCopy }
    'repo-authority-audit' { Invoke-RepositoryAuthority -Mode 'Audit' }
    'repo-authority-prepare' { Invoke-RepositoryAuthority -Mode 'Prepare' }
    'repo-authority-publish' { Invoke-RepositoryAuthority -Mode 'Publish' -Confirmation $PublishConfirmation }
}
