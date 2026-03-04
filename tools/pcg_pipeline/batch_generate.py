"""
Codename: Subspace — PCG Pipeline — Batch Generate

Single-command universe generation that orchestrates all sub-generators
and writes output to a structured build directory.

Usage (CLI):
    python -m pcg_pipeline --seed SEED --systems N --output-dir DIR

Usage (library):
    from pcg_pipeline.batch_generate import generate_universe
    result = generate_universe(seed=123456, num_systems=5,
                               output_dir='build/pcg')
"""

import argparse
import json
import os
import sys

from . import galaxy_generator
from . import ship_generator as ship_gen


def generate_universe(seed=123456, num_systems=5, output_dir="build/pcg"):
    """Generate a full Subspace universe.

    Args:
        seed: Universe master seed.
        num_systems: Number of star systems.
        output_dir: Root output directory for all generated data.

    Returns:
        dict with the full galaxy data.
    """
    galaxy = galaxy_generator.generate_galaxy(seed, num_systems)

    # Ensure output directories exist
    dirs = ["systems", "planets", "stations", "ships", "characters"]
    for d in dirs:
        os.makedirs(os.path.join(output_dir, d), exist_ok=True)

    # Write galaxy manifest
    galaxy_path = os.path.join(output_dir, "galaxy.json")
    with open(galaxy_path, "w") as fh:
        json.dump(galaxy, fh, indent=2)
        fh.write("\n")

    # Write per-system, per-asset JSON files
    for system in galaxy["systems"]:
        sid = system["system_id"]

        sys_path = os.path.join(output_dir, "systems", f"{sid}.json")
        with open(sys_path, "w") as fh:
            json.dump(system, fh, indent=2)
            fh.write("\n")

        for planet in system["planets"]:
            p_path = os.path.join(output_dir, "planets",
                                  f"{planet['planet_id']}.json")
            with open(p_path, "w") as fh:
                json.dump(planet, fh, indent=2)
                fh.write("\n")

        for station in system["stations"]:
            st_path = os.path.join(output_dir, "stations",
                                   f"{station['station_id']}.json")
            with open(st_path, "w") as fh:
                json.dump(station, fh, indent=2)
                fh.write("\n")

        for ship in system["ships"]:
            ship_gen.save_ship_metadata(
                ship, os.path.join(output_dir, "ships"))

        for char in system["characters"]:
            c_path = os.path.join(output_dir, "characters",
                                  f"{char['char_id']}.json")
            with open(c_path, "w") as fh:
                json.dump(char, fh, indent=2)
                fh.write("\n")

    return galaxy


def main(argv=None):
    """CLI entry-point for batch generation."""
    parser = argparse.ArgumentParser(
        description="Codename: Subspace — PCG Pipeline — "
                    "Batch Universe Generator")
    parser.add_argument("--seed", type=int, default=123456,
                        help="Universe master seed (default: 123456)")
    parser.add_argument("--systems", type=int, default=5,
                        help="Number of star systems (default: 5)")
    parser.add_argument("--output-dir", default="build/pcg",
                        help="Output directory (default: build/pcg)")
    args = parser.parse_args(argv)

    print(f"Subspace Generator — generating universe "
          f"(seed={args.seed}, systems={args.systems})")

    galaxy = generate_universe(
        seed=args.seed,
        num_systems=args.systems,
        output_dir=args.output_dir,
    )

    total_planets = sum(len(s["planets"]) for s in galaxy["systems"])
    total_stations = sum(len(s["stations"]) for s in galaxy["systems"])
    total_ships = sum(len(s["ships"]) for s in galaxy["systems"])
    total_chars = sum(len(s["characters"]) for s in galaxy["systems"])

    print(f"✓ Generated {len(galaxy['systems'])} systems, "
          f"{total_planets} planets, {total_stations} stations, "
          f"{total_ships} ships, {total_chars} characters")
    print(f"✓ Output written to {os.path.abspath(args.output_dir)}/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
