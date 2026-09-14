# R0-R1 — Rust Rewrite Bootstrap

## R0: project onboarding

This bootstrap establishes:

- Rust workspace
- `CodenameSubspace.emberproject`
- `forge.project.v1` project descriptor
- project-owned PCC
- separate-repository integration boundaries for Ember, Forge and Cortex
- donor/parity directories

## R1: editor-shell contracts

The bootstrap also establishes the Subspace-owned side of the editor boundary:

- six primary workspaces
- real panel descriptors with dock hints
- contextual tool registry
- stable command registry
- canonical document/session/selection/history structures
- `ShipBlueprint`
- stable IDs

The `subspace_ember_bridge` crate intentionally depends on **Subspace contracts only**.
Ember implements the host side in the Ember repository. This prevents Subspace from
depending on Ember's private crate topology and preserves independent buildability.

## Next

The Ember lane should implement the host adapter and render these descriptors through
its existing docking/panel system. Then replace the fixture-only Ship Viewport with a
real Rust viewport displaying `ShipBlueprint`.
