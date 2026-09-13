# Codename Subspace — World Continuity, Travel & Interior Authority

**Status:** Current architecture authority  
**Scope:** Space, atmosphere, planets, persistent ships, interior cells, docking, in-system travel and interstellar gates.

## Player-facing rule

Normal exploration is continuous.

No visible scene load for:
- space -> atmosphere;
- atmosphere -> surface;
- landing/takeoff;
- surface -> atmosphere -> space;
- changing between landed ship and surface vehicles/on-foot movement.

Approved diegetic loaded cells:
- station hangars/drydocks;
- building interiors;
- underground elevators/mines/bunkers;
- sealed authored POIs;
- airlocks/vestibules/transit chambers.

Interstellar gates use a visible warp tunnel. The tunnel may perform destination streaming but is experienced as travel, not a loading screen.

## Persistent ship invariant

A ship retains one stable ShipId and canonical assembly/state across:
- planet surface;
- atmosphere;
- orbit;
- system space;
- in-system travel;
- jump-gate travel;
- docking/hangar;
- save/reload.

The game may change LOD, physics proxy, render cluster or parent frame. It may not silently replace the ship with unrelated state.

## Travel states

1. Local Flight
2. In-System Travel Drive
3. Jump-Gate Transit
4. Docked/Hangar

These are states of the same ship.

## World hierarchy

Galaxy
-> Solar System
-> Celestial Body / Orbital Frame
-> Atmosphere
-> Planet Surface Streaming Regions
-> Local High-Fidelity Bubble
-> Interior/POI Cell where required

Moving through the hierarchy changes fidelity/reference frame, not identity.

## Interior cell model

Each cell owns:
- CellId
- ParentWorldId
- EntranceAnchorId
- ExitAnchorId
- persistence policy
- object/NPC/loot deltas
- quest/faction/security state
- destruction/repair state where supported

The exterior parent remains authoritative while the cell is loaded.

## Docking and Shipyard

Docking physically ends in a hangar/berth/drydock. The hangar is a transition boundary and service gameplay space.

The in-game Shipyard modifies the actual docked persistent ship using the same assembly schemas and validators that Ember uses during development.

## Surface world

Planetary surfaces are continuous logical open worlds internally partitioned for streaming.

Supported traversal targets:
- foot
- R.I.G.
- hover bike
- rover
- utility/mining vehicle
- mech
- atmospheric shuttle
- atmosphere-capable spacecraft

Not every spacecraft must support atmosphere or landing.

## Reference Solar System acceptance

The first production reference system must certify:
- seamless surface-to-space;
- persistent ship landing/takeoff;
- multiple surface traversal modes;
- at least one loaded building interior;
- at least one elevator/shaft underground cell;
- station hangar docking;
- in-game Shipyard editing the actual ship;
- in-system travel drive;
- interstellar gate + warp tunnel;
- same ship exiting another system;
- persistence/save/reload across all transitions.
