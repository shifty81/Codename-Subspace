# NullHarbor R183 — Planetary Command Historical Materializer Retirement

## Failure closed by this pass

After R182 restored the certified September 30 Studio/runtime source and all Studio R33-R82R2 gates passed, the project-local PCC still invoked the older R158-R177 Planetary Command text materializer before CMake/CTest. The old transformer then rejected the already-modern `PlayerFacingIntegrationSystem.h` because its historical preimage no longer existed.

That is migration-authority drift, not a C++ gameplay failure.

## R183 authority

R179 Planetary Command source is the canonical implementation. R182 remains the certified-baseline recovery for the seven files that had been overlaid from the older validation tree. R183 retires the historical R158-R177 replay path after proving the current semantic authorities.

During the existing Studio normalized-state proof, the R182 recovery tool now verifies the R179 Planetary Command model across economy, input, native window, player-facing integration, application, renderer, and workspace owners. Only after those checks pass does it write all known historical Planetary Command materialization marker names under `artifacts/migrations/`.

The compatibility entrypoint `scripts/subspace_planetary_command_r158_r177_apply.ps1` is also verification-only. If an older PCC revision invokes it directly, it verifies the same canonical R179 source and writes the markers; it never attempts historical exact/indent-normalized text transforms.

No C++ source is changed by R183. No Studio gate is weakened. Unknown/incomplete Planetary Command source fails closed before a materialization marker is emitted.
