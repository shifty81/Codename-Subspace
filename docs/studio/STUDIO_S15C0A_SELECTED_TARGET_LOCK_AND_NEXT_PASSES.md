# S15C0A — Studio selected-target lock and integration continuity

Baseline: `cee9993c48f7b0d3c854096c763a77577760b63b`. This source change is a **small first interaction pass**, not the entire Studio/PCC/Cortex/Blender program. The earlier research package, if approved, may coexist; it is not a prerequisite for this source fix.

## Implemented by the opt-in source diff

- In standalone Studio, only **Select** picks a different placed module on a viewport body click. Move/Rotate/Scale operate on the existing selected target via the gizmo; a miss neither selects nor starts free manipulation.
- Apply the same restriction to both in-game ShipBuilder press paths and both release-click fallback picking paths. Catalog selection, dock click capture, staged placement and explicit Outliner/UI selection remain allowed.
- No new independent selection store, command registry, editor executable, PCC or AI provider. Shared `StudioToolInteractionPolicy.h` encodes the rule and has a standalone compile smoke. No game flight-camera behavior changed.

## Applying this two-stage root package

1. Drop the `.patch` at repository root unextracted; use existing Subspace PCC to preview/approve/apply. The package adds only new files and does **not** overwrite edited game sources during intake.
2. From the root run `tools\handoffs\ApplyS15C0A.cmd -CheckOnly`, then `tools\handoffs\ApplyS15C0A.cmd`. Exact Git blob preimages and `git apply --check` refuse unexpected working files or commits lacking the source baseline. Do NOT bypass a failed preflight.
3. Run the Subspace FULL QUALITY GATE, then manually test: select hull A; activate Rotate; click hull B or empty viewport and verify A remains selected and neither moves; rotate gizmo A across B and release; Escape cancels; switch Select and choose B; use Outliner to switch; try Move/Scale; repeat in-game ShipBuilder. Test catalog drag/staged placement and floating-panel occlusion. Then certify and separately commit/push only if green.

## Remaining work — tracked, not implemented by S15C0A

- Studio gesture capture across focus/capture loss, durable stable object selection identity and undo consolidation; currently only accidental tool-mode reselection is addressed.
- S15C0B: geometry-aware snap and hull/interior penetration, authorized hatch/cavity conditions, validate before commit, PCG parity.
- S15C1: cursor/surface focus, close orbit/adaptive near clip, full viewport projection/picking authority, 3D tool gizmos, selected-mesh outline/measurements.
- S16: normalize UI chrome/docking, tools/icons, Outliner/Inspector/Asset Browser, command/shortcut authority.
- S17: Kitbash Workbench; source-stack mesh import/separate/slice/UV/animation/socket authoring, BlenderDCCAdapter interactive + headless worker, .blend source + GLB derivatives + Subspace sidecar, version/provenance.
- S18: material PBR/paint-zone normalization, validated animated hatch exemplar, user-approved source/asset intake.
- S19: physically valid ships/stations with portals/interior/FPS run, save/reload and PCG proof.
- External integration P0/P1: Subspace PCC project-owned authority preserved; Forge universal operation IDs/leases/events; Cortex persistent intelligent client, ForgeGUI as external shared presentation, per-project CLI for fallback. No unapproved ForgePY/Rust takeover. Cortex-to-Studio request must be versioned, target document revision+stable IDs, stage generated geometry, validate hull/material/animation, user approve before publication. Blender optional dev dependency, not runtime.

Status at packaging: policy smoke and source fixture `git apply` preflight may be run here, but native full project build, runtime manual checks, and local PCC GREEN must be performed in the user's exact Windows tree. Do not declare this pass GREEN before that receipt.
