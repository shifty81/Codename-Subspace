# Codename Subspace — Ember Hosted Project Contract

**Status:** Current integration contract; Ember implementation remains future/deferred.  
**Principle:** Ember is an external authoring host; Codename Subspace remains the independent runtime, data, validation, build, and certification authority.

## Non-negotiable boundaries

Subspace MUST build, run, test, patch, certify, save, and migrate without Ember. Subspace owns canonical schemas, validators, persistence formats, gameplay implementation, project-local CLI/PCC commands, and runtime launch profiles.

Ember MUST NOT vendor/fork the Subspace runtime, maintain shadow canonical data, silently reinterpret project schemas, or become required for headless/server/build workflows.

## Current operations boundary

The **internal standalone PCC** is the active Subspace operational authority. Forge/Cortex integration is deferred until explicitly certified and promoted. Ember therefore invokes project-owned commands exposed through `project.control.json`; it does not duplicate patch intake, Full Gate, Git publication, packaging, or debug-bundle authority.

## Discovery contract

Ember discovers a versioned project descriptor/capability contract rather than hardcoding `project == Subspace`. Required concepts include:

- contract/version identity;
- project identity/version;
- runtime/launch profiles;
- command authority;
- `GameData/` runtime-data root and `content/` governed-metadata roots;
- schema/capability registries;
- workspace extensions;
- importers/exporters;
- validators;
- debug launch/context profiles;
- artifact/log/debug-bundle locations.

## Capability families

Initial capabilities include system/celestial editing, continuous planet/surface-region editing, ship/station assembly editing, interiors/cells, faction/security/territory, economy/markets/production, encounters/missions/quests, items/fitting, materials/livery/decals, PCG, validation, runtime launch-at-context, and debug collection.

## Canonical schema families

- stable identity and schema versioning;
- hierarchical spatial frame;
- celestial/system and continuous planet/surface-region data;
- persistent interior/POI cell + entrance anchors;
- BuildElement/Assembly/SubAssembly;
- semantic socket/attachment face;
- material/livery/decal;
- ship class/hull/role/lineage;
- module/item/fitting/resource networks;
- faction/security/territory;
- economy/market/recipe;
- settlement/survey/exploration records;
- encounter/mission/quest;
- PCG grammar/generation receipt;
- validation results and runtime launch context.

## Shared validation rule

Forbidden state:

`EditorValid == true` while the Subspace runtime/compiler rejects the same canonical record.

Editor save/round-trip uses the same schema versions and project-owned semantic validators used by runtime/gates.

## Shipyard integration

Ember's generic assembly workspace should expose the same canonical assembly model used by the in-game Shipyard: selection, semantic sockets/faces, quantized/local grids, transforms, parametric elements, authored modules, nested subassemblies, symmetry, paint/material/decal layers, typed connections, undo/redo commands, validation diagnostics, and compile preview.

The in-game Shipyard operates under ownership/facility/skill/resource constraints on the actual persistent docked ship; Ember is the unrestricted developer-authoring surface over the same data model.

## Reference Solar System workflow

Required end-to-end interoperability fixture:

Open Subspace
-> Open **Reference Solar System**
-> edit system/planet region/ship/station/interior/settlement/encounter
-> validate with project-owned validators
-> launch runtime at selected context
-> collect logs/debug evidence
-> return to Ember
-> confirm canonical round trip
-> run project gate when requested.

“Golden Home System” is retired terminology and must not reappear in active Ember/Subspace contracts.

## Certification threshold

Ember/Subspace interoperability reaches player/developer-usable maturity only after clean-checkout discovery, schema negotiation, canonical save/round-trip, runtime loading of the same data, launch-at-context, returned diagnostics, and independent Subspace certification without Ember are all demonstrated.
