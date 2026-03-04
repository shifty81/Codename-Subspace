# Codename: Subspace — Tools

This directory contains utilities to help developers and modders create
and validate custom content for Codename: Subspace.

## Available Tools

### validate_json.py

JSON validation tool for checking game data files.

**Usage:**
```bash
# Validate all JSON files in GameData/
python tools/validate_json.py

# Validate with verbose output
python tools/validate_json.py --verbose

# Validate a single file
python tools/validate_json.py --file GameData/Quests/combat_pirates.json

# Validate a specific directory
python tools/validate_json.py --data-dir custom_mod/data
```

**Features:**
- Syntax validation (checks for valid JSON)
- Structure validation (checks for required fields per file type)
- Value range validation (warns about unusual values)
- Color-coded output for easy reading

**Supported file types:**
- **Quests** (`GameData/Quests/*.json`) — validates name, type, level,
  objectives, and rewards
- **Tutorials** (`GameData/Tutorials/*.json`) — validates name, steps,
  and category
- **Block definitions** (`block_definitions.json`) — validates block
  names, categories, hit points, and mass

### contract_scan.py

Scans the C++ engine source for forbidden API usage that would break
determinism in simulation code.

**Usage:**
```bash
# Scan the default engine/ directory
python tools/contract_scan.py

# Scan a different root
python tools/contract_scan.py --path engine
```

**Features:**
- Detects non-deterministic time sources (`std::chrono`, `time()`)
- Detects non-deterministic randomness (`rand()`, `srand()`, `<random>`)
- Only scans simulation directories (ECS, physics, AI, procedural,
  navigation, combat, ships)
- Returns exit code 1 when violations are found (CI-friendly)

### create_block.py

Interactive block creation tool that guides you through creating a new
block definition.

**Usage:**
```bash
python tools/create_block.py
```

**Features:**
- Interactive prompts for all block properties
- Category selection menu
- Default values for quick creation
- Resource cost definition
- Power consumption/generation settings
- Placement rule configuration
- JSON output preview
- Auto-save to file with overwrite protection

## Using with Make

All tools are available as Makefile targets:

```bash
make validate           # Validate all GameData JSON files
make validate-verbose   # Validate with verbose output
make contract-scan      # Scan C++ engine for contract violations
make check-deps         # Check if build dependencies are installed
```

## Future Tools

The following tools are planned for future releases:

### mission_editor.py (Planned)
GUI or CLI tool for creating and editing quest files with templates.

### create_module.py (Planned)
Interactive tool for creating ship module JSON definitions.

### balance_analyzer.py (Planned)
Analyzes block and ship stats to identify balance issues.

### mod_packager.py (Planned)
Packages mods into distributable archives with metadata.

## Contributing

Have an idea for a modding tool? Check out the
[Contributing Guide](../CONTRIBUTING.md) to get started!
