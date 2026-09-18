# Codename Subspace — seamless universe and diegetic fleet command direction

Status: product/architecture design authority only. The G3 GUI patch packaged with this document does **not** implement seamless planetary traversal, a new game-space streamer, command-seat multiplayer, or an RTS simulation. These are acceptance requirements for subsequent implementation, not claims of features already delivered.

## One continuous game universe

Ships, ship interiors, stations and their interiors, orbital space, atmospheric approach, planetary surfaces, vehicles and EVA share one coherent world identity and simulation. The player should be able to board a ship, walk to a station, depart, fly to a planet, descend, land and walk on terrain without changing to an unrelated minigame, duplicating campaign authority or opening a separate ship-combat simulation. Streaming and level-of-detail transitions are implementation details: do not reset object IDs, ownership, damage, inventory, fleet orders, persistent structures, or player state across them. Transition effects may hide streaming, but should not substitute an unrelated world instance. Home safe-area and expedition risk zones become policy/security partitions of the same simulation, not independent campaign authorities. Interstellar travel may stream destination star systems while preserving continuity and state.

World architecture boundaries: global high-precision frame / sector and body coordinates; local floating origins and physics islands; authoritative persistent entity IDs and transform lineage; async planet/terrain/orbit/interior streaming; deterministic PCG seed + persisted deltas; region-based replication and interest management; cross-boundary navigation, sensors and long-range commands. Planet radius and surface extent should be credible at a strategic scale and must not require fully generated surfaces in memory. Distance, time warp (if eventually allowed), flight and orbital mechanics need separate audited simulation contracts.

## Fleet command is a physical station, not a disconnected strategy screen

A Fleet Command Terminal can be installed in a station command room, capital vessel or compatible ship bridge. A player must physically access an authorized seat/terminal, sit/activate it, and enter command view; their avatar remains located at that seat. The same fleet-order service underlies cockpit fleet view and a remote terminal. A pilot may enter fleet view from their own pilot seat only when the relevant ship systems/permissions allow it. A safely located station offers tactical distance, but is only safe while its location and links remain safe; command links can be disrupted and terminal permissions must be checked.

Command view is a projection of the **live, same universe**: fleets, ships, stations, planets, friend/foe status, sensor fidelity, formation and route orders, mining/logistics, engagement rules and docking. It does not spawn separate combat entities or freeze the rest of the world. Order execution belongs to existing fleet/AI systems and an authoritative multiplayer simulation, not to the UI or camera. Crew seat / turret / pilot / fleet terminal claims require explicit control ownership, permissions and handoff, and a reliable return to first-person at the original seat. Remote turrets use separately delegated aim/fire capability; being in fleet view does not give unrestricted weapon control.

Suggested command interfaces: tactical map/3D fleet view, ship hierarchy, order queue and acknowledgments, sensor/contact confidence, status/notifications, permissions, and escape/exit to seated first-person. Sensor/communication range and latency must affect what can be observed/ordered; never expose omniscient location of unexplored/hidden hostiles. A pilot cannot both steer via mouse look and manipulate RTS controls: switching profiles must hand input authority off explicitly.

## Studio vs game integration

Subspace Studio is the full authoring environment with Shipyard, Station Forge, Vehicle Forge, Planet Forge, World/Galaxy, materials, PCG, interiors and logic as focused workspaces over shared documents and engine contracts. `subspace_game.exe` remains immersive first-person on foot and cockpit flight, and exposes constrained in-game crafting/construction and the physically accessed Fleet Command Terminal. Do not copy the full authoring GUI into every runtime mode. The PCC builds, tests and packages all relevant targets from one project authority.

## Delivery order and acceptance

1. Finish GUI baseline, real File/Edit/View/Help actions, asset browse/scroll/view modes, paint, Blender-style editor navigation, blank startup and trustworthy save/open. Preserve separate FPS and cockpit controllers in gameplay.
2. Complete usable 3D construction with one pick/render/transform authority, interior-only/cutaway, generated-and-edited interiors, doors and collision parity; persist edited documents and re-open them.
3. Extract Studio application boundary without duplicating engine, command, asset, document and simulation authority.
4. Establish seamless-world contracts and a small vertical slice: EVA -> boarded ship -> station dock -> ship departure -> atmospheric approach -> landing -> on-foot planet, with stable identity/save/collision, while fleet orders continue running.
5. Install the first terminal and cockpit fleet-command view on one shared fleet order bus; test seat occupation, remote command, authority handoff, network reconnection, and return to FPS.
6. Scale planet streaming, stations, multiplayer fleets and the original ship-class/variant catalog only after the slice is stable.

Do not report this roadmap as implemented by the G3 patch. The current G3 pass adds GUI, camera navigation and *preview* visibility modes only, with documented limits for physical interior editing and save parity.
