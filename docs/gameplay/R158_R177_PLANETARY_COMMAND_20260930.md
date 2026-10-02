# R158-R177 Planetary Command

This tranche converts the existing flat Planetary Industry flower into the first Planetary Command surface without building the project.

## Locked contract
The planet is the world. Hexes are stable physical surface-sector identities over the globe, not a detached PI board.

- UNKNOWN: no sector overlay.
- FRONTIER: faint expansion candidate adjacent to controlled territory.
- SURVEYED: intelligence known, not owned.
- CLAIMED: player/corporation controlled.
- DEVELOPED: claimed and hosting active infrastructure.

## Pass ledger
- R158: visible workspace becomes PLANETARY COMMAND.
- R159: persistent claim/development/owner state.
- R160: deterministic axial-hex -> spherical projection.
- R161: far hemisphere culling.
- R162: territory overlay.
- R163: resource overlay.
- R164: hazard/buildability overlay.
- R165: logistics adjacency overlay.
- R166: landing-candidate overlay.
- R167: development/installations overlay.
- R168: selected-sector inspector.
- R169: reuse existing RMB-orbit / wheel-zoom strategic camera.
- R170: LMB projected-sector hit testing.
- R171: UP/DOWN visible-sector selection.
- R172: F5 overlay cycling.
- R173: ENTER surveys a frontier sector.
- R174: ENTER claims a surveyed contiguous sector.
- R175: SHIFT+ENTER preserves tether/elevator industrialization.
- R176: legacy industrial placement auto-claims as a compatibility fallback and marks the sector developed.
- R177: source authority + persistent landing bridge contract.

## Territory expansion
The first surveyed buildable sector can establish the initial claim. Later claims must share a hex edge with existing controlled territory. Surveying is not ownership, and ownership is not development.

## Planet presentation
Only surveyed, controlled/developed, and immediate frontier sectors are projected on the visible hemisphere. The whole planet is not covered by a permanent honeycomb.

## Surface/landing bridge
Durable sector identity is `planetId + axial(q,r)`. Landing/descent must consume that same selected sector, eventually giving:

Planetary Command -> selected controlled sector -> landing site -> descent -> landed cockpit -> streamed physical sector.
