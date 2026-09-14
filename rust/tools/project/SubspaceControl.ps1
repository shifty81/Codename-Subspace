param(
    [ValidateSet("menu","status","build","test","run","shipyard","server","full-gate","debug-bundle","git-commit-green")]
    [string]$Action = "menu"
)

$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$Artifacts = Join-Path $Root "artifacts"
$Logs = Join-Path $Artifacts "logs"
$Gates = Join-Path $Artifacts "gates"
$Debug = Join-Path $Artifacts "debug"
New-Item -ItemType Directory -Force -Path $Logs,$Gates,$Debug | Out-Null

function Write-Header {
    Write-Host ""
    Write-Host "========================================================================"
    Write-Host " CODENAME SUBSPACE RUST PROJECT CONTROL CENTER"
    Write-Host "========================================================================"
    Write-Host " Root      : $Root"
    Write-Host " Authority : Project-owned PCC / forge.project.v1"
    Write-Host " Runtime   : Rust rewrite / Ember-hosted editor"
    Write-Host "------------------------------------------------------------------------"
}

function Require-Command([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Required command '$Name' was not found on PATH."
    }
}

function Run-Cargo([string[]]$Args) {
    Require-Command "cargo"
    Write-Host "[RUN] cargo $($Args -join ' ')"
    & cargo @Args
    if ($LASTEXITCODE -ne 0) { throw "cargo $($Args -join ' ') failed with exit code $LASTEXITCODE" }
}

function Write-Gate([bool]$Passed, [string]$Message) {
    $record = [ordered]@{
        schema = "subspace.rust.gate.v1"
        timestamp = (Get-Date).ToString("o")
        result = if($Passed){"PASS"}else{"FAIL"}
        message = $Message
        gitHead = ""
    }
    if (Get-Command git -ErrorAction SilentlyContinue) {
        Push-Location $Root
        try { $record.gitHead = (& git rev-parse HEAD 2>$null) } catch {}
        Pop-Location
    }
    $record | ConvertTo-Json -Depth 5 | Set-Content -Encoding UTF8 (Join-Path $Gates "latest.json")
}

function Project-Status {
    Write-Header
    if (Get-Command rustc -ErrorAction SilentlyContinue) { rustc --version } else { Write-Host "[WARN] rustc not found" }
    if (Get-Command cargo -ErrorAction SilentlyContinue) { cargo --version } else { Write-Host "[WARN] cargo not found" }
    if (Get-Command git -ErrorAction SilentlyContinue) {
        Push-Location $Root
        try { git status --short --branch } finally { Pop-Location }
    }
}

function Full-Gate {
    Write-Header
    $passed = $false
    try {
        Require-Command "cargo"
        Run-Cargo @("fmt","--all","--","--check")
        Run-Cargo @("clippy","--workspace","--all-targets","--","-D","warnings")
        Run-Cargo @("test","--workspace")
        Run-Cargo @("build","--workspace")
        $passed = $true
        Write-Gate $true "Rust workspace full gate certified."
        Write-Host "[PASS] FULL QUALITY GATE"
    }
    catch {
        Write-Gate $false $_.Exception.Message
        Write-Host "[FAIL] FULL QUALITY GATE: $($_.Exception.Message)"
        throw
    }
}

function Debug-Bundle {
    $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $staging = Join-Path $Debug "staging-$stamp"
    $zip = Join-Path $Debug "Subspace_Rust_DebugBundle_$stamp.zip"
    New-Item -ItemType Directory -Force -Path $staging | Out-Null

    foreach($rel in @("Cargo.toml","rust-toolchain.toml","project.control.json","CodenameSubspace.emberproject","README.md")) {
        $src = Join-Path $Root $rel
        if(Test-Path $src){ Copy-Item $src (Join-Path $staging ($rel -replace '[\\/]', '_')) }
    }
    if(Test-Path (Join-Path $Gates "latest.json")) { Copy-Item (Join-Path $Gates "latest.json") $staging }
    if(Test-Path $Logs){ Copy-Item $Logs (Join-Path $staging "logs") -Recurse -Force }
    Compress-Archive -Path (Join-Path $staging "*") -DestinationPath $zip -Force
    Remove-Item $staging -Recurse -Force
    Write-Host "[PASS] Debug bundle: $zip"
}

function Commit-Green {
    Require-Command "git"
    $gate = Join-Path $Gates "latest.json"
    if(-not (Test-Path $gate)){ throw "No Full Gate record found. Run option 1 first." }
    $record = Get-Content $gate -Raw | ConvertFrom-Json
    if($record.result -ne "PASS"){ throw "Latest Full Gate is not PASS." }

    Push-Location $Root
    try {
        git add -A
        $pending = git status --porcelain
        if(-not $pending){
            Write-Host "[PASS] Nothing to commit."
            return
        }
        $message = "Codename Subspace Rust: certified green"
        git commit -m $message
        if($LASTEXITCODE -ne 0){ throw "git commit failed" }
        git push
        if($LASTEXITCODE -ne 0){ throw "git push failed" }
        Write-Host "[PASS] Committed and pushed current GREEN."
    }
    finally { Pop-Location }
}

function Menu {
    while($true){
        Write-Header
        Write-Host " 1. FULL QUALITY GATE / CERTIFY GREEN"
        Write-Host " 2. COMMIT + PUSH CURRENT GREEN"
        Write-Host ""
        Write-Host " 3. Run game"
        Write-Host " 4. Run Shipyard"
        Write-Host " 5. Run tests"
        Write-Host " 6. Project status / health"
        Write-Host " 7. Package debug bundle"
        Write-Host " 8. Run dedicated server"
        Write-Host " 0. Exit"
        Write-Host ""
        $choice = Read-Host "Select"
        try {
            switch($choice){
                "1" { Full-Gate }
                "2" { Commit-Green }
                "3" { Run-Cargo @("run","-p","subspace_game") }
                "4" { Run-Cargo @("run","-p","subspace_shipyard_app","--","content/fixtures/sample_ship_blueprint.json") }
                "5" { Run-Cargo @("test","--workspace") }
                "6" { Project-Status }
                "7" { Debug-Bundle }
                "8" { Run-Cargo @("run","-p","subspace_server") }
                "0" { return }
                default { Write-Host "[WARN] Unknown selection." }
            }
        } catch { Write-Host "[FAIL] $($_.Exception.Message)" }
        Write-Host ""
        Read-Host "Press Enter to continue"
    }
}

Push-Location $Root
try {
    switch($Action){
        "menu" { Menu }
        "status" { Project-Status }
        "build" { Run-Cargo @("build","--workspace") }
        "test" { Run-Cargo @("test","--workspace") }
        "run" { Run-Cargo @("run","-p","subspace_game") }
        "shipyard" { Run-Cargo @("run","-p","subspace_shipyard_app","--","content/fixtures/sample_ship_blueprint.json") }
        "server" { Run-Cargo @("run","-p","subspace_server") }
        "full-gate" { Full-Gate }
        "debug-bundle" { Debug-Bundle }
        "git-commit-green" { Commit-Green }
    }
}
finally { Pop-Location }
