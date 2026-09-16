# Shipyard First-Class Editor / ForgeGUI Convergence

**Pass:** PASS1454-1465  
**Successor to:** PASS1444-1453 DCC Asset Shell  
**ForgeGUI donor:** `shifty81/ForgeGUI_Core@532f7e1` (0.4.8 Creator Studio)

This tranche treats Shipyard as a first-class DCC editor rather than a collection of legacy Shipyard controls rendered inside a new shell. ForgeGUI is used as a design-contract donor: compact metrics, semantic surface hierarchy, structural panel headers, object-header/property-grid separation, slim rail behavior, and bottom-tray density are projected into native Subspace C++. The Rust/egui runtime is not embedded.

## Visible corrections

- Right-side Properties owns a dedicated panel header and selected-object header before any actions.
- Inspector actions are contextual: Transform, Assembly, Sockets, Authoring, and Appearance no longer compete for the same vertical space.
- The viewport header keeps navigation menus on the left and display/shading controls on the right.
- The Asset Browser shelf has one structural header, one filter/action line, one category line, then asset cards.
- Asset cards lead with friendly part names and demote semantic/role metadata.
- Long labels are width-aware and truncate before crossing card or panel bounds.
- ForgeGUI-derived semantic dark surfaces and compact metrics replace ad-hoc panel measurements.
- Windows standalone Shipyard requests dark non-client chrome and rounded corners when DWM supports them.
- The standalone window identifies itself as `Codename: Subspace - Shipyard Editor`.

## Tooling projection

The right inspector rail is the workflow selector. It exposes only the tools relevant to the active context:

- **Transform:** frame part/ship, snap, symmetry, socket overlay, shield preview.
- **Assembly:** validate, save blueprint/draft, apply/commit assembly.
- **Sockets:** socket navigation, add/remove, overlay, save socket overrides.
- **Authoring:** semantic assignment, generator eligibility, pairing, save definition overrides.
- **Appearance:** livery and paint/decal actions.

The existing authoritative Shipyard command system remains underneath these projections; this pass reorganizes access rather than forking functionality.

## Universal editor direction

The shell contracts remain dimension/domain neutral. Ship modules are the first live consumer, while station, interior, character, material, celestial, VFX and UI authoring can reuse the same viewport/rail/outliner/properties/browser/status composition through adapters.
