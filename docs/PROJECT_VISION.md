# Codename Subspace Project Vision

Codename Subspace is an embodied persistent space sandbox, strategy game and RPG in which the player can personally fly, walk, salvage, mine, build, explore and trade while growing into fleet, industry and corporate command.

## Identity

Subspace is not a clone or a collection of disconnected reference-game mechanics. Its defining rule is systemic continuity: persistent ships, geography, exploration information, settlements, factions, industry, logistics, security, territory and the economy all participate in one simulation.

## World continuity

Normal travel is seamless from system space through orbit and atmosphere to large open planetary surfaces. Surface-to-space takeoff has no ordinary loading scene. Dense building interiors, deep underground POIs, mine/elevator shafts, bunkers and station hangars may use diegetic loaded cells. Interstellar jump gates mask destination streaming through a visible warp tunnel.

## Scale targets

- Ideal full solar-system in-system crossing: roughly **2 real minutes minimum**.
- Average major planet pole-to-pole atmospheric flight: roughly **10 real minutes** under good conditions.
- Ground-vehicle planetary traversal: days-scale.
- Walking planetary traversal: weeks-scale.

PCG must respect travel-time topology instead of uniformly sprinkling POIs.

## Physical traversal

On foot, R.I.G., hover bike, rover, utility/mining vehicle, mech, atmospheric shuttle, and atmosphere-capable spacecraft all occupy meaningful traversal roles. Not every spacecraft must be atmosphere/landing capable.

## Persistent ships

One ShipId and canonical assembly/state survive surface landing, atmospheric flight, orbit, in-system travel, jump-gate transit, docking, hangar service, save/reload and Shipyard refit. The in-game Shipyard modifies the actual docked ship.

## Tooling

Subspace remains independently buildable/testable/runnable through its internal PCC. Ember will load Subspace as an external first-class authoring project against the same canonical schemas and validators.
