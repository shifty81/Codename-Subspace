param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")),
    [switch]$Apply
)

$rules = @(
    @{ From = "Assets"; To = "content/assets"; Purpose = "source and runtime art assets" },
    @{ From = "assets"; To = "content/assets"; Purpose = "lowercase asset mirror" },
    @{ From = "GameData"; To = "content/data"; Purpose = "gameplay data, definitions, tuning" }
)

foreach ($rule in $rules) {
    $from = Join-Path $Root $rule.From
    $to = Join-Path $Root $rule.To
    if (Test-Path $from) {
        Write-Host "PLAN: $($rule.From) -> $($rule.To) [$($rule.Purpose)]"
        if ($Apply) {
            New-Item -ItemType Directory -Force -Path $to | Out-Null
            Write-Host "Apply mode currently creates target directories only. Move/copy should be done after manifest review."
        }
    }
}
