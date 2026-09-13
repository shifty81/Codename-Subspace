# Codename Subspace — Alignment Authority, 2026-09-12

This file is the current operational/design bridge for the September 12 normalization work. It supersedes any earlier chat-era statement that made ForgePY mandatory for Subspace patch intake. Forge/Cortex integration is deferred until explicitly promoted. The project-owned PCC is authoritative in the interim.

## Identity

Codename Subspace is an embodied persistent space sandbox, strategy game, and RPG. The player can personally fly, walk, explore, salvage, mine, build and fight while progressing into fleet, industry and corporate command. Nullharbor is retained design/system lineage folded into Subspace rather than a parallel game. External games are references only; accepted ideas are normalized into Subspace terminology and systems.

## World continuity

Normal traversal is seamless from system space through orbit, atmosphere and large streamed planetary surfaces. Planet entry/landing/takeoff are not loading scenes. Selected dense building interiors, underground shafts/mines/bunkers and similar authored spaces may use persistent loaded cells behind diegetic doors, elevators, airlocks or transition chambers. Docking/hangar entry is an intentional transition boundary.

## Persistent ships and Shipyard

A ship retains the same identity, assembly, cargo, damage, modules, crew, carried craft and engineering state while landed, in atmosphere, in orbit, during system travel, through jump-gate transit, while docked and across save/reload. The in-game Shipyard edits the actual persistent ship in its hangar/service berth. Ember later consumes the same canonical assembly schemas for developer authoring.

## Travel scale

- Ideal full-system in-system-drive crossing: approximately two real minutes minimum.
- Average major-planet pole-to-pole atmospheric flight: approximately ten real minutes.
- Planet-scale rover travel: days-scale.
- Planet-scale walking: weeks-scale.
- Interstellar travel uses jump gates; the warp tunnel persists as long as needed to prepare the destination, then visibly decelerates before exit.

World generation must respect these travel scales rather than distributing POIs uniformly.

## Planetary settlement/exploration identity

Planets contain persistent settlements from field sites/outposts through towns, industrial complexes and cities. Settlements participate in faction ownership, security, population, production, logistics, territory, infrastructure, missions and changing states. Exploration produces persistent survey information whose value depends on novelty, quality, distance, danger, rarity, resource/scientific/strategic usefulness, freshness and exclusivity. Information may be sold, licensed, shared, auctioned or retained for the player's own corporation. Discovery can seed later resource claims, expeditions, infrastructure and settlements.

## Reference Solar System

“Golden Home System” is retired. The **Reference Solar System** is the first production-quality system used to certify seamless space/atmosphere/surface travel, planetary exploration, settlements, persistent ships, interiors/cells, hangar Shipyard, in-system drive, jump gates, economy, factions and persistence before broad galaxy expansion.

## Authoring/tooling

Subspace remains independently buildable/runnable/testable/certifiable. Ember must later be able to load Subspace as a first-class external project, but Ember is not a runtime dependency. Current project operations are owned by the internal PCC.


## Deep audit lock

Pass902–911 establishes the deep alignment audit as the current gap/maturity authority alongside the machine-readable completion registry. Detached historical source lanes are not automatically restored to the build; runtime state ownership must converge before broad new gameplay breadth; `GameData/` remains canonical authored runtime data while `content/` owns governed metadata/schema/provenance.
