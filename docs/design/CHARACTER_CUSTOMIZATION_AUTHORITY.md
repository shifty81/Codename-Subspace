# Character Customization Authority

## Goal

Subspace character customization should reach the **depth and directness associated with EVE Online's character creator** while remaining original and data-driven.

Useful reference characteristics from EVE include:

- direct on-model face/body sculpting rather than slider-only editing;
- a large morph/blend-shape vocabulary;
- separate structural shape, complexion/skin, eyes, hair, clothing and body-detail layers;
- tattoos, scars, piercings and augmentations;
- later recustomization that can expose cosmetic changes independently from deeper structural resculpting;
- portrait posing/background/camera presentation as part of character identity.

Historical CCP material described 92 face morph targets plus additional body morph targets. Subspace must **not** hard-code that count. The `CharacterDefinition` contract is data-driven and may scale to whatever morph library its authored characters require.

## Canonical data

`CharacterDefinition` owns:

- stable character identity;
- body archetype;
- body/head meshes;
- rig identity;
- skin material;
- morph channels;
- direct-sculpt zone bindings;
- eyes/hair/skin details;
- clothing/accessories;
- tattoos/scars/piercings/cybernetic augmentations;
- portrait pose/expression/background/camera/light state.

Animation and source-mesh authoring attach to the same character definition/rig identity rather than forming separate player-facing data.

## Shipyard Character workspace

Full developer/editor capability:

- front/profile/three-quarter direct sculpting;
- morph channel inspector;
- body proportion authoring;
- source mesh/head/body assignment;
- rig/skeleton inspection and binding;
- skin/material authoring;
- hair/eyes/brows/facial hair;
- tattoos/scars/makeup/piercings/augmentations;
- apparel slot definitions, variants and masks;
- armor/gear attachment sockets;
- animation preview/retarget;
- portrait pose, expression, camera and lighting;
- LOD/collision/clearance validation;
- NPC archetype/variant authoring.

## Game character creation

Player-facing initial creation consumes the same `CharacterDefinition`, but the policy normally exposes:

- approved body/face sculpt zones;
- skin/complexion;
- eyes;
- hair;
- skin detail;
- clothing/accessories available at creation;
- body modifications/augmentations allowed by game rules;
- portrait pose/expression/background.

It never exposes raw mesh import, rig editing, source assets, unsafe morph ranges or developer metadata.

## Later recustomization

Normal recustomization can freely expose cosmetics such as eyes, hair, skin detail, clothing, accessories, augmentations and portrait controls.

Structural body/face changes are controlled by an explicit **structural resculpt permission**. Subspace may tie that to gameplay, medical/cybernetic facilities, progression or economy later; the engine contract does not hard-code an EVE-style monetization model.

## Direct sculpt interaction

`CharacterSculptBinding` maps a named on-model interaction zone to one or more morph channels and horizontal/vertical drag weights.

That allows the UI to grab a nose, jaw, brow, mouth, cheek, shoulder, waist, hip, etc. and manipulate the underlying data without forcing the player to understand technical morph names.

Advanced users can still open the Morph Channels dock in the Shipyard editor.

## Runtime validation

A character cannot certify unless:

- schema version is supported;
- stable identity exists;
- required body/head/rig/skin assets exist;
- morph IDs are unique and values stay inside authored safe ranges;
- sculpt bindings reference valid morph channels;
- apparel/body-modification combinations satisfy authored compatibility rules;
- animation/retarget requirements match the active rig;
- player-facing policy never exposes editor-only rig/source operations.
