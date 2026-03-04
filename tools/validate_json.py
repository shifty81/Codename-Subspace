#!/usr/bin/env python3
"""
Codename: Subspace - JSON Validation Tool

Validates all JSON files in the GameData/ directory for syntax errors,
missing required fields, and common content mistakes.

Usage:
    python tools/validate_json.py
    python tools/validate_json.py --verbose
    python tools/validate_json.py --file GameData/Quests/combat_pirates.json
    python tools/validate_json.py --data-dir GameData
"""

import json
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Tuple


class Colors:
    """ANSI color codes for terminal output"""
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'


def print_success(msg: str):
    print(f"{Colors.OKGREEN}✓{Colors.ENDC} {msg}")


def print_error(msg: str):
    print(f"{Colors.FAIL}✗{Colors.ENDC} {msg}")


def print_warning(msg: str):
    print(f"{Colors.WARNING}⚠{Colors.ENDC} {msg}")


def print_info(msg: str):
    print(f"{Colors.OKBLUE}ℹ{Colors.ENDC} {msg}")


def validate_json_syntax(filepath: Path) -> Tuple[bool, str]:
    """
    Validate that a file contains valid JSON syntax.
    Returns (is_valid, error_message)
    """
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            json.load(f)
        return True, ""
    except json.JSONDecodeError as e:
        return False, f"Line {e.lineno}, Column {e.colno}: {e.msg}"
    except Exception as e:
        return False, str(e)


def validate_quest(data: Dict, filepath: Path) -> Tuple[List[str], List[str]]:
    """Validate a quest definition"""
    errors = []
    warnings = []

    required = ['Id', 'Title', 'Description', 'Objectives', 'Rewards']
    for field in required:
        if field not in data:
            errors.append(f"Missing required field: {field}")

    valid_difficulties = ['Easy', 'Normal', 'Hard', 'Heroic']
    if 'Difficulty' in data and data['Difficulty'] not in valid_difficulties:
        warnings.append(
            f"Difficulty '{data['Difficulty']}' not in standard values: "
            f"{valid_difficulties}"
        )

    if 'Objectives' in data:
        if not isinstance(data['Objectives'], list):
            errors.append("Objectives must be an array")
        elif not data['Objectives']:
            warnings.append("Objectives array is empty")
        else:
            for i, obj in enumerate(data['Objectives']):
                if isinstance(obj, dict):
                    if 'Description' not in obj and 'Type' not in obj:
                        warnings.append(
                            f"Objectives[{i}] has no 'Description' or 'Type'"
                        )

    if 'Rewards' in data:
        if not isinstance(data['Rewards'], list):
            errors.append("Rewards must be an array")
        else:
            for i, reward in enumerate(data['Rewards']):
                if isinstance(reward, dict):
                    if 'Type' not in reward:
                        warnings.append(f"Rewards[{i}] has no 'Type'")
                    if 'Amount' in reward:
                        amount = reward['Amount']
                        if not isinstance(amount, (int, float)) or amount < 0:
                            errors.append(
                                f"Rewards[{i}].Amount must be a non-negative number"
                            )

    if 'TimeLimit' in data:
        tl = data['TimeLimit']
        if not isinstance(tl, (int, float)) or tl < 0:
            errors.append("TimeLimit must be a non-negative number")

    return errors, warnings


def validate_tutorial(data: Dict, filepath: Path) -> Tuple[List[str], List[str]]:
    """Validate a tutorial definition"""
    errors = []
    warnings = []

    required = ['Id', 'Title', 'Description', 'Steps']
    for field in required:
        if field not in data:
            errors.append(f"Missing required field: {field}")

    if 'Steps' in data:
        if not isinstance(data['Steps'], list):
            errors.append("Steps must be an array")
        elif not data['Steps']:
            warnings.append("Steps array is empty")
        else:
            for i, step in enumerate(data['Steps']):
                if isinstance(step, dict):
                    if 'Title' not in step and 'Message' not in step:
                        warnings.append(
                            f"Steps[{i}] has no 'Title' or 'Message'"
                        )

    return errors, warnings


def validate_block_definitions(data, filepath: Path) -> Tuple[List[str], List[str]]:
    """Validate block definition data (expected as a JSON array)"""
    errors = []
    warnings = []

    if isinstance(data, dict):
        # Handle legacy dict format
        items = data.items()
    elif isinstance(data, list):
        items = [(block.get('id', f'index_{i}'), block) for i, block in enumerate(data)]
    else:
        errors.append("Block definitions must be a JSON array or object")
        return errors, warnings

    for block_id, block_data in items:
        if not isinstance(block_data, dict):
            errors.append(f"Block '{block_id}' must be an object")
            continue

        required = ['id', 'displayName']
        for field in required:
            if field not in block_data:
                errors.append(
                    f"Block '{block_id}' missing required field: {field}"
                )

        if 'hitPointsPerVolume' in block_data:
            hp = block_data['hitPointsPerVolume']
            if not isinstance(hp, (int, float)) or hp < 0:
                errors.append(
                    f"Block '{block_id}': hitPointsPerVolume must be non-negative"
                )

        if 'massPerUnitVolume' in block_data:
            mass = block_data['massPerUnitVolume']
            if not isinstance(mass, (int, float)) or mass < 0:
                errors.append(
                    f"Block '{block_id}': massPerUnitVolume must be non-negative"
                )

    return errors, warnings


def validate_file(filepath: Path, verbose: bool = False) -> Tuple[int, int]:
    """
    Validate a single JSON file.
    Returns (error_count, warning_count)
    """
    is_valid, error_msg = validate_json_syntax(filepath)
    if not is_valid:
        print_error(f"{filepath}: {error_msg}")
        return 1, 0

    errors = []
    warnings = []

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)

        path_str = str(filepath).lower()

        if 'quests/' in path_str or 'quest' in filepath.stem.lower():
            errs, warns = validate_quest(data, filepath)
            errors.extend(errs)
            warnings.extend(warns)
        elif 'tutorials/' in path_str or 'tutorial' in filepath.stem.lower():
            errs, warns = validate_tutorial(data, filepath)
            errors.extend(errs)
            warnings.extend(warns)
        elif 'block_definitions' in filepath.stem.lower():
            errs, warns = validate_block_definitions(data, filepath)
            errors.extend(errs)
            warnings.extend(warns)

    except Exception as e:
        print_error(f"{filepath}: Unexpected error: {e}")
        return 1, 0

    if errors:
        print_error(f"{filepath}: {len(errors)} error(s)")
        if verbose:
            for error in errors:
                print(f"    {error}")
    elif warnings:
        print_warning(f"{filepath}: {len(warnings)} warning(s)")
        if verbose:
            for warning in warnings:
                print(f"    {warning}")
    else:
        if verbose:
            print_success(f"{filepath}: Valid")

    return len(errors), len(warnings)


def main():
    parser = argparse.ArgumentParser(
        description='Validate Codename: Subspace JSON files'
    )
    parser.add_argument('--file', type=str, help='Validate a single file')
    parser.add_argument(
        '--verbose', '-v', action='store_true', help='Show detailed output'
    )
    parser.add_argument(
        '--data-dir', type=str, default='GameData',
        help='Data directory to scan (default: GameData)',
    )
    args = parser.parse_args()

    print(
        f"{Colors.BOLD}{Colors.HEADER}"
        f"Codename: Subspace - JSON Validation Tool"
        f"{Colors.ENDC}\n"
    )

    total_errors = 0
    total_warnings = 0
    total_files = 0

    if args.file:
        filepath = Path(args.file)
        if not filepath.exists():
            print_error(f"File not found: {filepath}")
            return 1

        errors, warnings = validate_file(filepath, args.verbose)
        total_errors += errors
        total_warnings += warnings
        total_files = 1
    else:
        data_dir = Path(args.data_dir)
        if not data_dir.exists():
            print_error(f"Data directory not found: {data_dir}")
            return 1

        json_files = sorted(data_dir.glob('**/*.json'))

        # Also check block_definitions.json in AvorionLike/
        block_defs = Path('AvorionLike/block_definitions.json')
        if block_defs.exists():
            json_files.append(block_defs)

        if not json_files:
            print_warning(f"No JSON files found in {data_dir}")
            return 0

        print_info(f"Scanning {len(json_files)} JSON file(s)\n")

        for filepath in json_files:
            errors, warnings = validate_file(filepath, args.verbose)
            total_errors += errors
            total_warnings += warnings
            total_files += 1

    print(f"\n{Colors.BOLD}Summary:{Colors.ENDC}")
    print(f"  Files checked: {total_files}")

    if total_errors == 0:
        print_success("No errors found")
    else:
        print_error(f"{total_errors} error(s) found")

    if total_warnings > 0:
        print_warning(f"{total_warnings} warning(s) found")

    return 1 if total_errors > 0 else 0


if __name__ == '__main__':
    sys.exit(main())
