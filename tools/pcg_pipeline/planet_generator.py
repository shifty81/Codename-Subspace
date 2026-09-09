"""
Codename: Subspace — PCG Pipeline — Planet Generator

Generates planet metadata using seed-based determinism and realistic
astronomical parameters.  Planet type, size, biome, orbit, and atmosphere
are derived from star type and orbital position.
"""

import math
import random

# Planet archetypes based on real exoplanet classifications
PLANET_TYPES = {
    "terrestrial":  {"radius_range": (0.5, 2.0),  "temp_range": (200, 350)},
    "super_earth":  {"radius_range": (2.0, 4.0),  "temp_range": (180, 400)},
    "gas_giant":    {"radius_range": (4.0, 15.0), "temp_range": (50, 300)},
    "ice_giant":    {"radius_range": (3.0, 10.0), "temp_range": (50, 200)},
}

# Biome thresholds driven by equilibrium temperature
_BIOME_THRESHOLDS = [
    (350, "volcanic"),
    (300, "desert"),
    (270, "temperate"),
    (230, "forest"),
    (180, "tundra"),
    (0,   "ice"),
]

# Atmosphere templates
_ATMOSPHERE_TEMPLATES = {
    "terrestrial": {"N2": 78, "O2": 21, "CO2": 1},
    "super_earth": {"N2": 60, "O2": 15, "CO2": 20, "Ar": 5},
    "gas_giant":   {"H2": 75, "He": 24, "CH4": 1},
    "ice_giant":   {"H2": 60, "He": 25, "CH4": 10, "NH3": 5},
}


def _equilibrium_temp(star_temp_k, orbit_au):
    """Simplified equilibrium temperature calculation.

    Uses a simplified radiative equilibrium model:
      T_eq ≈ T_star * sqrt(R_star / (2 * d))
    where R_star is approximated from T_star.
    """
    r_star_au = (star_temp_k / 5800.0) ** 2 * 0.00465
    if orbit_au <= 0:
        orbit_au = 0.1
    return star_temp_k * math.sqrt(r_star_au / (2.0 * orbit_au))


def _biome_from_temp(temp_k):
    """Map equilibrium temperature to a biome string."""
    for threshold, biome in _BIOME_THRESHOLDS:
        if temp_k >= threshold:
            return biome
    return "ice"


def generate_planet(seed, planet_id, star_type="G", orbit_index=0,
                    total_planets=5):
    """Generate planet metadata.

    Args:
        seed: Deterministic seed.
        planet_id: Unique identifier.
        star_type: Spectral class of the parent star.
        orbit_index: Ordinal position from the star (0 = closest).
        total_planets: Total number of planets in the system.

    Returns:
        dict with full planet metadata.
    """
    rng = random.Random(seed)

    # Orbit distance in AU — inner planets closer, outer farther
    base_au = 0.3 + (orbit_index / max(total_planets - 1, 1)) * 4.7
    orbit_au = base_au * rng.uniform(0.8, 1.2)

    # Star temperature lookup (default to G-type)
    from . import system_generator
    star_info = system_generator.STAR_TYPES.get(
        star_type, system_generator.STAR_TYPES["G"])
    star_temp = star_info["temp_k"]

    # Planet type — inner orbits favour rocky, outer favour gas
    if orbit_au < 1.5:
        ptype = rng.choice(["terrestrial", "terrestrial", "super_earth"])
    elif orbit_au < 3.0:
        ptype = rng.choice(["super_earth", "gas_giant", "terrestrial"])
    else:
        ptype = rng.choice(["gas_giant", "gas_giant", "ice_giant"])

    type_info = PLANET_TYPES[ptype]
    radius_earth = round(rng.uniform(*type_info["radius_range"]), 2)

    # Temperature and biome
    temp_k = round(_equilibrium_temp(star_temp, orbit_au), 1)
    biome = _biome_from_temp(temp_k)

    # Rotation period (hours) — rough range
    rotation_period_h = round(rng.uniform(8, 2000), 1)
    axial_tilt_deg = round(rng.uniform(0, 45), 1)

    # Orbital period (years) via Kepler's third law approximation
    luminosity = star_info["luminosity"]
    orbital_period_y = round(
        math.sqrt(orbit_au ** 3 / max(luminosity, 0.001)), 2)

    # Atmosphere
    atmosphere = dict(_ATMOSPHERE_TEMPLATES.get(ptype, {}))

    # Foliage (only for habitable biomes on rocky worlds)
    foliage = []
    if ptype in ("terrestrial", "super_earth") and biome in (
            "temperate", "forest"):
        foliage_density = rng.randint(50, 300)
        foliage_types = rng.sample(
            ["tree", "shrub", "grass", "rock", "flower"],
            k=min(3, rng.randint(1, 5)),
        )
        foliage = [{"type": ft, "density": foliage_density}
                   for ft in foliage_types]

    # Liquid bodies — only for worlds with suitable temperature
    liquids = {}
    if ptype in ("terrestrial", "super_earth") and 200 < temp_k < 350:
        sea_level = round(rng.uniform(0.02, 0.15), 3)
        liquids = {
            "sea_level": sea_level,
            "composition": "H2O" if temp_k >= 250 else "NH3",
            "coverage_pct": round(rng.uniform(10, 80), 1),
        }

    # Mineable resources (Subspace resource types found on this planet)
    # Use a separate RNG seeded with an offset so that resource selection
    # is independent of the foliage/liquid rolls above.
    _RESOURCE_SEED_OFFSET = 7
    resource_rng = random.Random(seed + _RESOURCE_SEED_OFFSET)
    all_resources = ["Iron", "Titanium", "Naonite", "Trinium",
                     "Xanion", "Ogonite", "Avorion"]
    # Rarer resources only on outer/exotic worlds
    max_rarity = min(len(all_resources), 2 + orbit_index)
    available = all_resources[:max_rarity]
    num_resources = resource_rng.randint(1, min(3, len(available)))
    resources = resource_rng.sample(available, k=num_resources)

    return {
        "planet_id": planet_id,
        "seed": seed,
        "type": ptype,
        "radius_earth": radius_earth,
        "orbit_au": round(orbit_au, 3),
        "temperature_k": temp_k,
        "biome": biome,
        "rotation_period_h": rotation_period_h,
        "axial_tilt_deg": axial_tilt_deg,
        "orbital_period_y": orbital_period_y,
        "atmosphere": atmosphere,
        "foliage": foliage,
        "liquids": liquids,
        "resources": resources,
    }
