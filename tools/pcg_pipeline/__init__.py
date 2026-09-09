"""
Codename: Subspace — PCG Pipeline

Seed-based procedural content generation pipeline for the Subspace universe.
Generates galaxies, star systems, planets, stations, ships, and NPCs
using deterministic seeds for fully reproducible content.

This pipeline can be used standalone (no game engine required) to
pre-generate universe metadata as JSON files, which the C++ engine and
C# prototype can load at runtime.

Usage (library):
    from pcg_pipeline import batch_generate
    batch_generate.generate_universe(seed=123456, num_systems=5,
                                     output_dir='build/pcg')

Usage (CLI):
    python -m pcg_pipeline --seed 123456 --systems 5 --output-dir build/pcg
"""

__version__ = "1.0.0"
__pipeline_name__ = "Subspace Generator"

from . import galaxy_generator
from . import system_generator
from . import planet_generator
from . import station_generator
from . import ship_generator
from . import character_generator
from . import batch_generate

__all__ = [
    "galaxy_generator",
    "system_generator",
    "planet_generator",
    "station_generator",
    "ship_generator",
    "character_generator",
    "batch_generate",
]
