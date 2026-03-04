"""
Codename: Subspace — PCG Pipeline — Galaxy Generator

Generates a deterministic galaxy structure from a universe seed.
Each galaxy contains star systems which in turn hold stars, planets,
stations, and ships.

All generation is seed-based for full reproducibility — the same seed
always produces the same galaxy.
"""

import random

from . import system_generator


def generate_galaxy(universe_seed, num_systems=5):
    """Generate a full galaxy with the given number of star systems.

    Args:
        universe_seed: Master seed for the universe.
        num_systems: Number of star systems to generate.

    Returns:
        dict with ``universe_seed`` and a list of ``systems``.
    """
    rng = random.Random(universe_seed)
    systems = []

    for i in range(num_systems):
        system_seed = rng.randint(0, 999999)
        system_id = f"system_{i:04d}"
        system = system_generator.generate_system(system_seed, system_id)
        systems.append(system)

    return {
        "universe_seed": universe_seed,
        "num_systems": num_systems,
        "systems": systems,
    }
