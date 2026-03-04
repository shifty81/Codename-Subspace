#!/usr/bin/env python3
"""
Codename: Subspace - Interactive Block Creator

Guides you through creating a new block JSON definition with all required
fields, following the format used in AvorionLike/block_definitions.json.

Usage:
    python tools/create_block.py
"""

import json
import sys
from pathlib import Path


def prompt(message, default=None, type_=str, choices=None):
    """Prompt user for input with optional default and validation"""
    if default is not None:
        message = f"{message} [{default}]"
    if choices:
        message = f"{message} ({'/'.join(choices)})"
    message += ": "

    while True:
        value = input(message).strip()
        if not value and default is not None:
            return default
        if not value:
            print("This field is required. Please enter a value.")
            continue

        if choices and value not in choices:
            print(f"Invalid choice. Please choose from: {', '.join(choices)}")
            continue

        if type_ == int:
            try:
                return int(value)
            except ValueError:
                print("Please enter a valid integer.")
                continue
        elif type_ == float:
            try:
                return float(value)
            except ValueError:
                print("Please enter a valid number.")
                continue

        return value


def main():
    print("=" * 60)
    print(" Codename: Subspace - Interactive Block Creator")
    print("=" * 60)
    print()
    print("This tool will guide you through creating a new block definition.")
    print("Press Ctrl+C at any time to cancel.")
    print()

    try:
        # Basic information
        print("=== Basic Information ===")
        block_id = prompt("Block ID (e.g., 'reinforced_hull')")
        name = prompt("Display Name (e.g., 'Reinforced Hull')")

        # Category
        print("\n=== Category ===")
        categories = [
            'Structure', 'Propulsion', 'Power', 'Defense',
            'Weapons', 'Utility', 'Systems',
        ]
        print("Available categories:")
        for i, cat in enumerate(categories, 1):
            print(f"  {i}. {cat}")
        cat_idx = prompt("Select category number", type_=int)
        if cat_idx < 1 or cat_idx > len(categories):
            print("Invalid category number. Using 'Structure'.")
            category = 'Structure'
        else:
            category = categories[cat_idx - 1]

        # Description
        print("\n=== Description ===")
        description = prompt("Block description")

        # Physical properties
        print("\n=== Physical Properties ===")
        hit_points = prompt("Hit Points", default=100, type_=int)
        mass = prompt("Mass (kg)", default=50.0, type_=float)

        # Resource costs
        print("\n=== Resource Cost ===")
        print("Enter resource costs. Press Enter with empty input when done.")
        resources = {}
        default_resources = {
            'Iron': 10,
            'Titanium': 5,
        }
        use_defaults = prompt(
            "Use default resources (Iron: 10, Titanium: 5)?",
            choices=['y', 'n'], default='y',
        )
        if use_defaults == 'y':
            resources = dict(default_resources)
        else:
            while True:
                res = input("Resource name (or press Enter to finish): ").strip()
                if not res:
                    break
                amount = prompt(f"  Amount of {res}", type_=int)
                resources[res] = amount

        # Power
        print("\n=== Power ===")
        power_consumption = prompt("Power consumption (MW)", default=0.0, type_=float)
        power_generation = prompt("Power generation (MW)", default=0.0, type_=float)

        # Placement rules
        print("\n=== Placement Rules ===")
        max_per_ship = prompt(
            "Max per ship (0 = unlimited)", default=0, type_=int
        )
        requires_adjacency = prompt(
            "Requires adjacency to hull?", choices=['y', 'n'], default='n'
        )

        # Build block dictionary
        block_data = {
            block_id: {
                "name": name,
                "category": category,
                "description": description,
                "hit_points": hit_points,
                "mass": mass,
                "resources": resources,
                "power_consumption": power_consumption,
                "power_generation": power_generation,
                "placement_rules": {
                    "max_per_ship": max_per_ship,
                    "requires_adjacency": requires_adjacency == 'y',
                },
            }
        }

        # Display result
        print("\n" + "=" * 60)
        print(" Block Definition Generated")
        print("=" * 60)
        print()
        print(json.dumps(block_data, indent=2))
        print()

        # Save option
        save = prompt("Save to file?", choices=['y', 'n'], default='y')
        if save == 'y':
            filename = prompt(
                "Output filename",
                default=f"GameData/blocks/{block_id}.json",
            )
            filepath = Path(filename)
            filepath.parent.mkdir(parents=True, exist_ok=True)

            if filepath.exists():
                overwrite = prompt(
                    f"{filename} exists. Overwrite?",
                    choices=['y', 'n'], default='n',
                )
                if overwrite != 'y':
                    print("Cancelled. Block definition not saved.")
                    return 0

            with open(filepath, 'w') as f:
                json.dump(block_data, f, indent=2)
                f.write('\n')

            print(f"\n✓ Block saved to {filename}")

    except KeyboardInterrupt:
        print("\n\nCancelled.")
        return 1
    except Exception as e:
        print(f"\nError: {e}")
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
