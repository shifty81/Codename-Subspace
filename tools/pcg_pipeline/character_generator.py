"""
Codename: Subspace — PCG Pipeline — Character Generator

Generates procedural NPC metadata.  Each character is assigned a faction
alignment, a role, and optional cybernetic augmentations.
"""

import random

# Factions matching the Subspace engine
FACTIONS = [
    "Iron Dominion",
    "Nomad Continuum",
    "Helix Covenant",
    "Ashen Clades",
    "Ascended Archive",
    "Unaffiliated",
]

# NPC roles
ROLES = [
    "Captain",
    "Trader",
    "Miner",
    "Engineer",
    "Mercenary",
    "Scientist",
    "Smuggler",
    "Diplomat",
]

# Augmentation slots
AUGMENT_SLOTS = [
    "left_arm", "right_arm", "left_leg", "right_leg",
    "torso_core", "spine", "optics",
]


def generate_character(seed, char_id):
    """Generate character metadata.

    Args:
        seed: Deterministic seed.
        char_id: Unique identifier string.

    Returns:
        dict with character metadata.
    """
    rng = random.Random(seed)

    faction = rng.choice(FACTIONS)
    role = rng.choice(ROLES)

    # Augmentations — more likely for certain roles
    augments = []
    aug_chance = 0.6 if role in ("Engineer", "Mercenary") else 0.3
    if rng.random() < aug_chance:
        num_augs = rng.randint(1, 3)
        augments = rng.sample(AUGMENT_SLOTS, k=min(num_augs,
                                                    len(AUGMENT_SLOTS)))

    return {
        "char_id": char_id,
        "seed": seed,
        "faction": faction,
        "role": role,
        "augmentations": augments,
    }
