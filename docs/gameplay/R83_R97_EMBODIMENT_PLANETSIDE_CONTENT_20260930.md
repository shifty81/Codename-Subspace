# R83-R112 Embodiment + Planet-Side Content Tranche

This tranche deliberately authors content before another native build cycle.

## What is added

- Standard first-person embodiment and interaction profiles.
- Nine reusable interior module definitions.
- A complete one-deck **Pioneer Frigate** starter interior with cockpit, utility core, crew bunk, airlock, cargo/salvage bay, engineering and docking collar.
- A connected portal graph and interaction placement set for the Pioneer.
- **Wayline K-17** station hangar/public-service content with two ship pads and service interactions.
- **Cinderwake**, a landable rocky planet archetype with five biomes, seven resource families and five landing-site templates.
- Five concrete planetary POI archetypes: abandoned survey outpost, cave mine, derelict lander, buried ruin and frontier drill site.
- Authored station docking/undocking and planet landing/takeoff transition sequences.
- The **First Descent** expedition chain tying ship interior, station, space, landing, exploration, salvage, mining, cave traversal, takeoff and return together.

## Runtime status

These are authored project content definitions. They are intentionally additive and do not claim runtime wiring yet. The next code tranche should consume these definitions through one content loader/registry rather than hardcoding equivalent gameplay data.

## Build policy for this tranche

No CMake/MSVC build is required just to accept the content files. A lightweight JSON/reference validator is included at `tools/content/validate_embodied_exploration_content.py` and the PowerShell wrapper is `scripts/subspace_r83_r97_content_validate.ps1`.

## Pass map

- R83: first-person embodiment profile
- R84: interaction and movement environments
- R85: reusable interior module set
- R86: Pioneer Frigate walkable interior
- R87: Pioneer portals and interactions
- R88: Wayline K-17 hangar and station interactions
- R89: Cinderwake planet archetype
- R90: biomes and terrain identity
- R91: resources
- R92: landing-site templates
- R93: ruins/outpost content
- R94: cave/mining content
- R95: salvage/industrial POIs
- R96: docking, descent, landing, takeoff and undocking transitions
- R97: First Descent end-to-end vertical-slice content registry

## R98-R112 expansion

The content-first tranche now also includes expedition items and loot, recoverable functional ship modules, station and surface NPC/drone archetypes, Cinderwake hazards, rover/hoverbike/mech vehicle definitions, cave and ruin room-generation templates, a persistent frontier-outpost template, repeatable contracts, a survey-data economy, planet spawn rules and ambient surface events.

This remains a no-build content authoring tranche. Runtime code should consume the catalog rather than duplicate these definitions.
