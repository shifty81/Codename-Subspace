"""
Codename: Subspace — PCG Pipeline — Ship Generator

Generates ship metadata using Subspace's ship archetypes, hull types,
factions, and module system.  Ship classes and factions align with the
C++ engine definitions in ``engine/include/ships/`` and
``engine/include/factions/``.

LOD (Level of Detail) tiers are embedded in every ship's metadata so that
the engine can request reduced-poly assets at distance:

  - **LOD0** — full detail, close-up / hangar view
  - **LOD1** — half poly-count, mid-range
  - **LOD2** — quarter poly-count, long range
"""

import json
import os
import random

# Ship classes matching engine/include/ships/ShipArchetype.h ArchetypeClass
SHIP_CLASSES = [
    "Interceptor",
    "Frigate",
    "Freighter",
    "Cruiser",
    "Battleship",
]

# Hull archetypes from engine/include/factions/FactionProfile.h HullArchetype
HULL_ARCHETYPES = ["Brick", "Needle", "Disk", "Hammerhead", "Carrier"]

# Subspace factions
FACTIONS = [
    "Iron Dominion",
    "Nomad Continuum",
    "Helix Covenant",
    "Ashen Clades",
    "Ascended Archive",
]

# Faction → preferred hull archetype (from FactionProfile.h)
FACTION_HULL_MAP = {
    "Iron Dominion":    "Brick",
    "Nomad Continuum":  "Needle",
    "Helix Covenant":   "Disk",
    "Ashen Clades":     "Hammerhead",
    "Ascended Archive": "Needle",
}

# Module types from engine/include/ships/ModuleDef.h
MODULE_POOL = ["Core", "Engine", "Weapon", "Hull", "Cargo", "Shield",
               "Utility"]

# Material types from engine/include/ships/Block.h
MATERIAL_TYPES = ["Iron", "Titanium", "Naonite", "Trinium", "Xanion",
                  "Ogonite", "Avorion"]

# LOD tier definitions
LOD_TIERS = [
    {"tier": 0, "poly_fraction": 1.0,  "max_distance": 500},
    {"tier": 1, "poly_fraction": 0.5,  "max_distance": 2000},
    {"tier": 2, "poly_fraction": 0.25, "max_distance": 8000},
]


def generate_ship_metadata(seed, ship_id):
    """Generate ship metadata JSON.

    Args:
        seed: Deterministic seed for this ship.
        ship_id: Unique identifier string.

    Returns:
        dict with ship metadata including LOD tiers.
    """
    rng = random.Random(seed)

    ship_class = rng.choice(SHIP_CLASSES)
    faction = rng.choice(FACTIONS)
    hull = FACTION_HULL_MAP.get(faction, rng.choice(HULL_ARCHETYPES))
    modules = rng.sample(MODULE_POOL,
                         k=rng.randint(2, min(5, len(MODULE_POOL))))
    hardpoints = rng.randint(0, 10)

    # Material tier correlates loosely with ship class
    class_tier = SHIP_CLASSES.index(ship_class)
    max_mat = min(len(MATERIAL_TYPES), class_tier + 3)
    material = rng.choice(MATERIAL_TYPES[:max_mat])

    return {
        "ship_id": ship_id,
        "seed": seed,
        "class": ship_class,
        "hull_archetype": hull,
        "faction": faction,
        "material": material,
        "modules": modules,
        "hardpoints": hardpoints,
        "lod_tiers": list(LOD_TIERS),
    }


def save_ship_metadata(ship_data, output_dir):
    """Persist ship metadata to a JSON file in *output_dir*.

    Args:
        ship_data: dict returned by :func:`generate_ship_metadata`.
        output_dir: Target directory.

    Returns:
        Path to the written JSON file.
    """
    os.makedirs(output_dir, exist_ok=True)
    path = os.path.join(output_dir, f"{ship_data['ship_id']}.json")
    with open(path, "w") as fh:
        json.dump(ship_data, fh, indent=2)
        fh.write("\n")
    return path
