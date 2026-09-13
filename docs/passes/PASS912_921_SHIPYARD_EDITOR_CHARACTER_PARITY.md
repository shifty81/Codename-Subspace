# Pass912-921 — Shipyard Editor / Character / Client Parity

## Purpose

Normalize Codename Subspace around exactly two native applications:

- **Shipyard Editor** — one Blender-like authoring environment and editor/engine shell.
- **Codename Subspace Client** — the player runtime.

They share canonical engine/data contracts. The editor is the authoring superset; the client exposes constrained player-facing editors without creating duplicate formats.

## Passes

- **Pass912** — one Shipyard editor application with Blender-like task workspaces.
- **Pass913** — Character authoring becomes a dock workspace inside the shared editor shell.
- **Pass914** — canonical `CharacterDefinition` with data-driven morphs/direct sculpt zones.
- **Pass915** — initial game character creation becomes a restricted view over the same definition.
- **Pass916** — ordinary recustomization separates cosmetics/apparel/body modifications from structural sculpting.
- **Pass917** — explicit structural-resculpt policy can unlock body/face changes without changing schema.
- **Pass918** — portrait-only and full editor authoring are permission profiles over one character type.
- **Pass919** — editor/game-client parity registry covers Ship, Character, World, Interior, Materials, Animation, PCG, VFX, Audio and Logic.
- **Pass920** — Carbon Engine open-source candidates are classified as integrate/evaluate/reference/defer instead of becoming a parallel engine.
- **Pass921** — project-wide normalization audit records character authority and parity authority.

## Verification

Portable focused build/tests:

- `SubspacePass912To921ShipyardEditorCharacterParityTests`: 10/10 PASS
- `SubspacePass892To901EditorDockNormalizationTests`: 10/10 PASS
- `SubspacePass555To584ProjectWideEditorStationUpgradeTests`: 30/30 PASS

The `subspace_engine` static library compiled successfully with the new files.

Windows project-owned PCC option **1** remains required before GREEN certification.

## Carbon adoption position

High-value evaluation candidates:

- Carbon Mesh — mesh/skeleton/animation pipeline candidate.
- Carbon Resources — resource delivery/tooling candidate.
- Carbon spatial-audio clustering — dense-scene audio optimization candidate.

Architecture references for now:

- Carbon Trinity — renderer/backend/shader architecture.
- Carbon Destiny — world simulation/benchmark architecture; current public build documentation still references private Perforce dependencies.

Deferred/conditional:

- Carbon IO — compare when multiplayer transport selection begins.
- Carbon Audio — evaluate after Subspace defines its backend abstraction; current project wraps Wwise.

No Carbon component is made a required Subspace dependency in this pass.
