# Pass1123-1162 — Core Authority Convergence

This tranche closes authority drift exposed during the generator/editor audit.

## Locked behavior

- Generate is deterministic from the visible canonical request. Reroll alone changes seed.
- A physically sane but topology-incomplete candidate may be exposed as a deterministic **DRAFT**. Unsafe giant-scale or collapsed candidates still fail closed.
- Shipyard translation defaults to **View/Camera** space. Ship heading and recipe forward orientation are removed before editing recipe-local coordinates. Ship and Local spaces remain explicit choices.
- Shipyard authoring has one global transaction history exposed by Undo/Redo. Drag/transform edits are grouped into committed operations instead of mouse-motion history spam.
- A native dockable **Forge / Project Tools** surface reads `forge.project.v1`. It does not embed ForgePY and never guesses an interpreter; `.ps1` operations must use their declared PowerShell executable.
- History and Forge panels are part of the shared dock workspace.

## Not claimed complete

The old showcase ship synthesizer and station exterior-stacking preview remain temporary candidate sources. This tranche protects users from corrupt promotion while the next player-scale/interior/topology generator is built.
