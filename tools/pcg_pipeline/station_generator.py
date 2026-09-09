"""
Codename: Subspace — PCG Pipeline — Station Generator

Generates procedural station metadata with modular sections, docking bays,
and faction-appropriate styling.  Station types and factions match the
Subspace C++ engine definitions.
"""

import random

# Station types matching the C++ GalaxyGenerator's GetRandomStationType()
STATION_TYPES = {
    "Trading":  {"base_modules": ["Core", "Cargo", "Hull"],
                 "style": "commercial"},
    "Military": {"base_modules": ["Core", "Weapon", "Shield"],
                 "style": "military"},
    "Mining":   {"base_modules": ["Core", "Cargo", "Engine"],
                 "style": "mining"},
    "Shipyard": {"base_modules": ["Core", "Hull", "Cargo"],
                 "style": "industrial"},
    "Research": {"base_modules": ["Core", "Utility", "Shield"],
                 "style": "research"},
    "Refinery": {"base_modules": ["Core", "Cargo", "Engine"],
                 "style": "industrial"},
}

# Subspace factions from engine/include/factions/FactionProfile.h
FACTIONS = [
    "Iron Dominion",
    "Nomad Continuum",
    "Helix Covenant",
    "Ashen Clades",
    "Ascended Archive",
]

# Module types matching engine/include/ships/ModuleDef.h
MODULE_POOL = ["Core", "Engine", "Weapon", "Hull", "Cargo", "Shield",
               "Utility"]


def generate_station(seed, planet_id, station_index):
    """Generate station metadata.

    Args:
        seed: Deterministic seed.
        planet_id: Parent planet identifier.
        station_index: Ordinal index among sibling stations.

    Returns:
        dict with station metadata.
    """
    rng = random.Random(seed)

    station_id = f"{planet_id}_station_{station_index:02d}"
    station_type = rng.choice(list(STATION_TYPES.keys()))
    template = STATION_TYPES[station_type]

    # Build module list from template + random extras
    modules = list(template["base_modules"])
    extra_count = rng.randint(0, 3)
    for _ in range(extra_count):
        modules.append(rng.choice(MODULE_POOL))

    faction = rng.choice(FACTIONS)
    docking_bays = rng.randint(1, 4)

    return {
        "station_id": station_id,
        "seed": seed,
        "planet_id": planet_id,
        "type": station_type,
        "style": template["style"],
        "faction": faction,
        "modules": modules,
        "docking_bays": docking_bays,
    }
