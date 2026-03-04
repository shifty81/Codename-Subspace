"""
Codename: Subspace — PCG Pipeline — System Generator

Generates a single star system containing stars, planets, stations, and
ships.  Uses realistic stellar classification (O, B, A, F, G, K, M) to
drive planet generation parameters.
"""

import random

from . import planet_generator
from . import station_generator as station_gen
from . import ship_generator as ship_gen
from . import character_generator as char_gen

# Spectral types with approximate temperature and colour (sRGB 0-255)
STAR_TYPES = {
    "O": {"temp_k": 30000, "color": [155, 176, 255], "luminosity": 30000},
    "B": {"temp_k": 20000, "color": [170, 191, 255], "luminosity": 2500},
    "A": {"temp_k": 8500,  "color": [202, 215, 255], "luminosity": 25},
    "F": {"temp_k": 6500,  "color": [248, 247, 255], "luminosity": 3.2},
    "G": {"temp_k": 5800,  "color": [255, 244, 234], "luminosity": 1.0},
    "K": {"temp_k": 4500,  "color": [255, 210, 161], "luminosity": 0.4},
    "M": {"temp_k": 3000,  "color": [255, 204, 111], "luminosity": 0.04},
}

# Realistic spectral type distribution (weighted towards K/M)
_STAR_WEIGHTS = ["M", "M", "M", "M", "K", "K", "K", "G", "G", "F", "A", "B"]


def generate_system(system_seed, system_id):
    """Generate a complete star system.

    Args:
        system_seed: Deterministic seed for this system.
        system_id: Unique identifier string.

    Returns:
        dict describing the system with stars, planets, stations, ships,
        and characters.
    """
    rng = random.Random(system_seed)

    # Stars
    star_type = rng.choice(_STAR_WEIGHTS)
    stars = [{
        "star_id": f"{system_id}_star_0",
        "type": star_type,
        "seed": rng.randint(0, 999999),
        "temperature_k": STAR_TYPES[star_type]["temp_k"],
        "color": STAR_TYPES[star_type]["color"],
        "luminosity": STAR_TYPES[star_type]["luminosity"],
    }]

    # Planets — count influenced by star type
    max_planets = 8 if star_type in ("G", "K", "F") else 5
    num_planets = rng.randint(1, max_planets)
    planets = []
    for i in range(num_planets):
        planet_seed = rng.randint(0, 999999)
        planet = planet_generator.generate_planet(
            planet_seed,
            planet_id=f"{system_id}_planet_{i:02d}",
            star_type=star_type,
            orbit_index=i,
            total_planets=num_planets,
        )
        planets.append(planet)

    # Stations — attached to suitable planets
    stations = []
    for planet in planets:
        num_stations = rng.randint(0, 2)
        for j in range(num_stations):
            station_seed = rng.randint(0, 999999)
            station = station_gen.generate_station(
                station_seed,
                planet_id=planet["planet_id"],
                station_index=j,
            )
            stations.append(station)

    # Ships
    num_ships = rng.randint(2, 6)
    ships = []
    for k in range(num_ships):
        ship_seed = rng.randint(0, 999999)
        ship = ship_gen.generate_ship_metadata(
            ship_seed,
            ship_id=f"{system_id}_ship_{k:02d}",
        )
        ships.append(ship)

    # Characters (NPCs)
    num_characters = rng.randint(1, 4)
    characters = []
    for c in range(num_characters):
        char_seed = rng.randint(0, 999999)
        character = char_gen.generate_character(
            char_seed,
            char_id=f"{system_id}_char_{c:02d}",
        )
        characters.append(character)

    return {
        "system_id": system_id,
        "seed": system_seed,
        "stars": stars,
        "planets": planets,
        "stations": stations,
        "ships": ships,
        "characters": characters,
    }
