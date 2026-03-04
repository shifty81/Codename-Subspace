"""
Validation tests for the Codename: Subspace PCG Pipeline.

These tests run without any game engine and verify the deterministic
generation logic, data structures, and cross-module integration.

Usage:
    python tools/pcg_pipeline/test_pcg_pipeline.py
"""

import json
import os
import sys
import tempfile

# Ensure the parent package is importable when running as a script
_here = os.path.dirname(os.path.abspath(__file__))
_tools_root = os.path.dirname(_here)
if _tools_root not in sys.path:
    sys.path.insert(0, _tools_root)

from pcg_pipeline import galaxy_generator
from pcg_pipeline import system_generator
from pcg_pipeline import planet_generator
from pcg_pipeline import station_generator
from pcg_pipeline import ship_generator
from pcg_pipeline import character_generator
from pcg_pipeline import batch_generate


# ── helpers ───────────────────────────────────────────────────────────

def _assert(condition, msg):
    if not condition:
        raise AssertionError(msg)


# ── individual tests ──────────────────────────────────────────────────

def test_galaxy_determinism():
    """Same seed must produce identical galaxies."""
    print("Testing galaxy determinism...")
    g1 = galaxy_generator.generate_galaxy(42, num_systems=3)
    g2 = galaxy_generator.generate_galaxy(42, num_systems=3)
    _assert(json.dumps(g1, sort_keys=True) == json.dumps(g2, sort_keys=True),
            "Galaxy output differs for the same seed")
    _assert(len(g1["systems"]) == 3, "Expected 3 systems")
    print("✓ Galaxy determinism verified")
    return True


def test_system_structure():
    """System must contain required top-level keys."""
    print("\nTesting system structure...")
    system = system_generator.generate_system(12345, "test_sys")
    required = {"system_id", "seed", "stars", "planets", "stations",
                "ships", "characters"}
    missing = required - set(system.keys())
    _assert(not missing, f"System missing keys: {missing}")
    _assert(len(system["stars"]) >= 1, "System must have at least one star")
    _assert(len(system["planets"]) >= 1,
            "System must have at least one planet")
    print(f"✓ System has {len(system['planets'])} planets, "
          f"{len(system['stations'])} stations, "
          f"{len(system['ships'])} ships")
    return True


def test_star_types():
    """All generated stars must use valid spectral classes."""
    print("\nTesting star type validity...")
    valid_types = set(system_generator.STAR_TYPES.keys())
    for seed in range(100):
        system = system_generator.generate_system(seed, f"sys_{seed}")
        for star in system["stars"]:
            _assert(star["type"] in valid_types,
                    f"Invalid star type: {star['type']}")
    print("✓ All star types valid across 100 seeds")
    return True


def test_planet_biomes():
    """Planets must have valid biomes and atmosphere data."""
    print("\nTesting planet generation...")
    valid_biomes = {"volcanic", "desert", "temperate", "forest",
                    "tundra", "ice"}
    for seed in range(50):
        p = planet_generator.generate_planet(
            seed, f"planet_{seed}", star_type="G",
            orbit_index=2, total_planets=6)
        _assert(p["biome"] in valid_biomes,
                f"Invalid biome: {p['biome']}")
        _assert(p["radius_earth"] > 0, "Radius must be positive")
        _assert(p["orbit_au"] > 0, "Orbit must be positive")
        _assert(isinstance(p["atmosphere"], dict),
                "Atmosphere must be a dict")
    print("✓ Planet biomes and parameters valid across 50 seeds")
    return True


def test_planet_foliage_and_liquids():
    """Temperate/forest terrestrial planets should have foliage and liquids."""
    print("\nTesting planet foliage and liquids...")
    found_foliage = False
    found_liquid = False
    for seed in range(200):
        p = planet_generator.generate_planet(
            seed, f"planet_{seed}", star_type="G",
            orbit_index=1, total_planets=5)
        if p["foliage"]:
            found_foliage = True
            for entry in p["foliage"]:
                _assert("type" in entry and "density" in entry,
                        "Foliage entry must have type and density")
        if p["liquids"]:
            found_liquid = True
            _assert("sea_level" in p["liquids"],
                    "Liquids must have sea_level")
    _assert(found_foliage, "No foliage generated across 200 seeds")
    _assert(found_liquid, "No liquids generated across 200 seeds")
    print("✓ Foliage and liquid generation verified")
    return True


def test_planet_resources():
    """Planets must have valid Subspace resource types."""
    print("\nTesting planet resources...")
    valid_resources = {"Iron", "Titanium", "Naonite", "Trinium",
                       "Xanion", "Ogonite", "Avorion"}
    for seed in range(50):
        p = planet_generator.generate_planet(
            seed, f"planet_{seed}", star_type="G",
            orbit_index=3, total_planets=6)
        _assert(isinstance(p["resources"], list),
                "resources must be a list")
        _assert(len(p["resources"]) >= 1,
                "Planet must have at least one resource")
        for r in p["resources"]:
            _assert(r in valid_resources,
                    f"Invalid resource: {r}")
    print("✓ Planet resources valid across 50 seeds")
    return True


def test_station_generation():
    """Station must have required fields and valid type."""
    print("\nTesting station generation...")
    valid_types = set(station_generator.STATION_TYPES.keys())
    for seed in range(50):
        s = station_generator.generate_station(seed, "planet_0", 0)
        _assert(s["type"] in valid_types,
                f"Invalid station type: {s['type']}")
        _assert(len(s["modules"]) >= 3,
                "Station must have at least base modules")
        _assert(s["faction"] in station_generator.FACTIONS,
                f"Invalid faction: {s['faction']}")
    print("✓ Station generation valid across 50 seeds")
    return True


def test_ship_generation():
    """Ship metadata must have required fields."""
    print("\nTesting ship generation...")
    for seed in range(50):
        s = ship_generator.generate_ship_metadata(seed, f"ship_{seed}")
        _assert(s["class"] in ship_generator.SHIP_CLASSES,
                f"Invalid class: {s['class']}")
        _assert(s["faction"] in ship_generator.FACTIONS,
                f"Invalid faction: {s['faction']}")
        _assert(s["hull_archetype"] in ship_generator.HULL_ARCHETYPES,
                f"Invalid hull: {s['hull_archetype']}")
        _assert(s["material"] in ship_generator.MATERIAL_TYPES,
                f"Invalid material: {s['material']}")
        _assert(0 <= s["hardpoints"] <= 10, "Hardpoints out of range")
        _assert(len(s["modules"]) >= 2,
                "Ship must have at least 2 modules")
    print("✓ Ship generation valid across 50 seeds")
    return True


def test_character_generation():
    """Character metadata must have valid faction/role/augmentations."""
    print("\nTesting character generation...")
    for seed in range(50):
        c = character_generator.generate_character(seed, f"char_{seed}")
        _assert(c["faction"] in character_generator.FACTIONS,
                f"Invalid faction: {c['faction']}")
        _assert(c["role"] in character_generator.ROLES,
                f"Invalid role: {c['role']}")
        for aug in c["augmentations"]:
            _assert(aug in character_generator.AUGMENT_SLOTS,
                    f"Invalid augment slot: {aug}")
    print("✓ Character generation valid across 50 seeds")
    return True


def test_batch_generate():
    """Batch generate must produce all expected output files."""
    print("\nTesting batch generation...")
    with tempfile.TemporaryDirectory() as tmpdir:
        galaxy = batch_generate.generate_universe(
            seed=42, num_systems=2, output_dir=tmpdir)

        _assert(os.path.isfile(os.path.join(tmpdir, "galaxy.json")),
                "galaxy.json not created")

        for system in galaxy["systems"]:
            sid = system["system_id"]
            _assert(os.path.isfile(
                os.path.join(tmpdir, "systems", f"{sid}.json")),
                f"System file for {sid} not created")

            for planet in system["planets"]:
                _assert(os.path.isfile(
                    os.path.join(tmpdir, "planets",
                                 f"{planet['planet_id']}.json")),
                    f"Planet file for {planet['planet_id']} not created")

            for ship in system["ships"]:
                _assert(os.path.isfile(
                    os.path.join(tmpdir, "ships",
                                 f"{ship['ship_id']}.json")),
                    f"Ship file for {ship['ship_id']} not created")

        with open(os.path.join(tmpdir, "galaxy.json")) as fh:
            loaded = json.load(fh)
        _assert(loaded["universe_seed"] == 42, "Seed mismatch in output")
        _assert(len(loaded["systems"]) == 2, "System count mismatch")

    print("✓ Batch generation produced all expected files")
    return True


def test_batch_determinism():
    """Two batch runs with the same seed must produce identical output."""
    print("\nTesting batch determinism...")
    with tempfile.TemporaryDirectory() as d1, \
         tempfile.TemporaryDirectory() as d2:
        g1 = batch_generate.generate_universe(
            seed=99, num_systems=3, output_dir=d1)
        g2 = batch_generate.generate_universe(
            seed=99, num_systems=3, output_dir=d2)
        _assert(
            json.dumps(g1, sort_keys=True) ==
            json.dumps(g2, sort_keys=True),
            "Batch outputs differ for same seed")
    print("✓ Batch determinism verified")
    return True


def test_ship_lod_tiers():
    """Ship metadata must include LOD tier data."""
    print("\nTesting ship LOD tiers...")
    s = ship_generator.generate_ship_metadata(42, "lod_test")
    _assert("lod_tiers" in s, "Ship metadata missing lod_tiers")
    _assert(len(s["lod_tiers"]) >= 3, "Need at least 3 LOD tiers")
    for tier in s["lod_tiers"]:
        _assert("tier" in tier, "LOD tier missing 'tier' field")
        _assert("poly_fraction" in tier,
                "LOD tier missing 'poly_fraction'")
        _assert("max_distance" in tier,
                "LOD tier missing 'max_distance'")
        _assert(0 < tier["poly_fraction"] <= 1.0,
                f"poly_fraction out of range: {tier['poly_fraction']}")
    print(f"✓ Ship has {len(s['lod_tiers'])} LOD tiers with valid fields")
    return True


def test_pipeline_package_imports():
    """pcg_pipeline package should export all sub-modules."""
    print("\nTesting pipeline package imports...")
    import pcg_pipeline
    expected = [
        "galaxy_generator", "system_generator", "planet_generator",
        "station_generator", "ship_generator", "character_generator",
        "batch_generate",
    ]
    for name in expected:
        _assert(hasattr(pcg_pipeline, name),
                f"pcg_pipeline missing attribute: {name}")
    _assert(hasattr(pcg_pipeline, "__version__"),
            "pcg_pipeline missing __version__")
    print(f"✓ All {len(expected)} sub-modules accessible via pcg_pipeline")
    return True


# ── runner ────────────────────────────────────────────────────────────

def run_tests():
    """Execute all PCG pipeline tests."""
    print("=" * 60)
    print("Codename: Subspace — PCG Pipeline Validation Tests")
    print("=" * 60)

    tests = [
        ("Galaxy Determinism", test_galaxy_determinism),
        ("System Structure", test_system_structure),
        ("Star Types", test_star_types),
        ("Planet Biomes", test_planet_biomes),
        ("Planet Foliage & Liquids", test_planet_foliage_and_liquids),
        ("Planet Resources", test_planet_resources),
        ("Station Generation", test_station_generation),
        ("Ship Generation", test_ship_generation),
        ("Character Generation", test_character_generation),
        ("Batch Generate", test_batch_generate),
        ("Batch Determinism", test_batch_determinism),
        ("Ship LOD Tiers", test_ship_lod_tiers),
        ("Pipeline Package Imports", test_pipeline_package_imports),
    ]

    results = []
    for name, func in tests:
        try:
            result = func()
            results.append((name, result))
        except Exception as exc:
            print(f"\n✗ {name} raised exception: {exc}")
            import traceback
            traceback.print_exc()
            results.append((name, False))

    print("\n" + "=" * 60)
    print("Test Results Summary:")
    print("=" * 60)

    passed = sum(1 for _, r in results if r)
    total = len(results)

    for name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status}: {name}")

    print("=" * 60)
    print(f"Total: {passed}/{total} tests passed")

    if passed == total:
        print("✓ All PCG pipeline tests passed!")
    else:
        print("✗ Some tests failed")

    print("=" * 60)
    return passed == total


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
