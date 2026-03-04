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
        hit_points = prompt("Hit Points Per Volume", default=100, type_=int)
        mass = prompt("Mass Per Unit Volume", default=1.0, type_=float)
        scalable = prompt("Scalable?", choices=['y', 'n'], default='y')

        # Function
        print("\n=== Function ===")
        functions = [
            'structure', 'protection', 'propulsion', 'maneuvering',
            'power', 'shields', 'cargo', 'crew', 'weapons', 'sensors',
            'docking', 'navigation',
        ]
        print("Available functions:")
        for i, func in enumerate(functions, 1):
            print(f"  {i}. {func}")
        func_idx = prompt("Select function number", type_=int)
        if func_idx < 1 or func_idx > len(functions):
            print("Invalid function number. Using 'structure'.")
            function = 'structure'
        else:
            function = functions[func_idx - 1]

        # Resource costs
        print("\n=== Resource Cost ===")
        print("Enter resource costs. Press Enter with empty input when done.")
        resources = {}
        default_resources = {
            'Iron': 10,
        }
        use_defaults = prompt(
            "Use default resources (Iron: 10)?",
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
        power_generation = prompt(
            "Power generation per volume", default=0.0, type_=float
        )
        power_consumption = prompt(
            "Power consumption per volume", default=0.0, type_=float
        )

        # Placement rules
        print("\n=== Placement ===")
        requires_internal = prompt(
            "Requires internal placement?", choices=['y', 'n'], default='n'
        )
        suitable_exterior = prompt(
            "Suitable for exterior?", choices=['y', 'n'], default='y'
        )
        min_tech = prompt("Minimum tech level", default=1, type_=int)
        ai_priority = prompt(
            "AI placement priority (1-10)", default=5, type_=int
        )

        # Color
        print("\n=== Appearance ===")
        default_color = prompt(
            "Default color (hex)", default="#3F3D3B"
        )

        # Build block definition matching project format
        block_data = {
            "id": block_id,
            "displayName": name,
            "blockType": 0,
            "description": description,
            "resourceCosts": resources,
            "hitPointsPerVolume": hit_points,
            "massPerUnitVolume": mass,
            "scalable": scalable == 'y',
            "function": function,
            "powerGenerationPerVolume": power_generation,
            "powerConsumptionPerVolume": power_consumption,
            "thrustPowerPerVolume": 0,
            "shieldCapacityPerVolume": 0,
            "cargoCapacityPerVolume": 0,
            "crewCapacityPerVolume": 0,
            "aiPlacementPriority": ai_priority,
            "requiresInternalPlacement": requires_internal == 'y',
            "suitableForExterior": suitable_exterior == 'y',
            "minTechLevel": min_tech,
            "defaultColor": default_color,
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
