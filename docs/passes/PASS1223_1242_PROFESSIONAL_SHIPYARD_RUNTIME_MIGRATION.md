# Pass1223-1242 — Professional Shipyard Runtime Migration

This tranche starts the strangler migration away from the monolithic Shipyard builder without destabilizing the certified runtime.

- `ShipyardBuilderMigrationSystem` projects the current builder into `ShipyardDocument + ShipyardSession`, preserves stable IDs for unchanged objects, and fingerprints authored state so session/status changes do not make the document dirty.
- `ShipyardProfessionalController` establishes stable command IDs as the UI/Cortex/Forge boundary.
- `ShipyardPanelModelSystem` produces independent stable-ID Outliner and context Properties models.
- `ShipyardJobSystem` establishes thread-safe Activity/Jobs state.
- `ShipyardCommandSystem` gains Advanced/Dev, Generate, New Seed + Generate, Explain Candidate and Validate Ship. Its registration path now captures the command key before moving the descriptor, fixing the latent moved-from-key bug that could collapse registrations into an empty map key.
- `ShipyardProfessionalUiSystem` gains Activity / Jobs and restores Character to the advanced menu.

`New Seed + Generate` explicitly runs the existing reroll command and then the same deterministic generation path used by Generate. Test remains a real workspace and is not aliased to Dev World.

The next tranche switches the visible Shipyard shell to these models and begins deleting duplicated panel/control code from `ShipyardBuilderSystem`.
