# Player Character Implementation — Superseded Historical Note

This document previously described an early C# prototype where the player's "character" was represented by a small ship/pod. That architecture is **not current Codename Subspace authority**.

Current authority:

- the player has an embodied humanoid/on-foot character;
- ships are persistent owned assemblies the character boards/pilots rather than the character entity itself;
- character authoring lives in the Character workspace of the single native Shipyard editor;
- player-facing character creation/customization consumes the same canonical `CharacterDefinition` under a restricted runtime policy;
- the old pod/hyperdrive/material progression references below are retained only in repository history and must not be reintroduced as runtime authority.

See:

- `docs/architecture/SHIPYARD_EDITOR_CLIENT_PARITY.md`
- `docs/design/CHARACTER_CUSTOMIZATION_AUTHORITY.md`
- `engine/include/character/CharacterCustomizationSystem.h`

This file intentionally replaces the previous prototype instructions so retrieval/search cannot mistake them for the current game design.
