# 🔨 ADVANCED SHIP BUILDING GUIDE
```
╔═══════════════════════════════════════════════════════════════════════════╗
║            CODENAME:SUBSPACE BLOCK STRETCHING & CONSTRUCTION              ║
║                    Craft Your Perfect Starship                            ║
╚═══════════════════════════════════════════════════════════════════════════╝
```

## 🎨 Overview

Ship building in Codename:Subspace goes beyond simple block placement. Our **Block Stretching System** allows you to create elaborate, efficient designs by dynamically sizing blocks along multiple axes. Every cubic meter matters!

---

## 🔲 Block Stretching Mechanics

### How It Works
```
1. Select Block Type & Material
   │
   ▼
2. Click to Set Start Position
   │
   ▼
3. Choose Stretch Axis (X, Y, Z, or combinations)
   │
   ▼
4. Drag to Desired Size
   │
   ▼
5. Release to Place
   │
   └─▶ Block automatically sized and costed
```

### Stretch Axes Available
```
┌──────────────┬────────────────────────────────────────┐
│  Axis Mode   │  Description                           │
├──────────────┼────────────────────────────────────────┤
│  X           │  Stretch horizontally (width)          │
│  Y           │  Stretch vertically (height)           │
│  Z           │  Stretch in depth                      │
│  XY          │  Stretch diagonally on horizontal plane│
│  XZ          │  Stretch flat (width + depth)          │
│  YZ          │  Stretch wall-like (height + depth)    │
│  XYZ         │  Stretch in all directions (cube)      │
└──────────────┴────────────────────────────────────────┘
```

### Visual Example
```
Start: 2×2×2 block
Stretch X: 2×2×2 ──────▶ 8×2×2

     ┌──┐                ┌────────┐
     │  │                │        │
     └──┘                └────────┘
   Original          Stretched along X-axis
```

---

## 📐 Size-Based Stat Scaling

### Core Principle
> **Volume = Power**
> Every stat scales proportionally with block volume (X × Y × Z)

### Scaling Formula
```
Block Volume = Size.X × Size.Y × Size.Z

All Stats Scale By:
  Stat = BaseStat × Volume × MaterialMultiplier
```

---

## ⚙️ Block Type Statistics

### 🛡️ Hull & Armor Blocks
```
┌────────────┬─────────────────────────────────────────┐
│  Property  │  Scaling                                │
├────────────┼─────────────────────────────────────────┤
│  HP        │  100 × Volume × Material Durability     │
│  Mass      │  Volume × Material Mass                 │
│  Cost      │  Volume × 10 × Material Tier            │
└────────────┴─────────────────────────────────────────┘

Special: Armor has 5× HP and 1.5× Mass of Hull
```

**Example**: 4×4×4 Titanium Hull
```
Volume: 64 m³
HP: 100 × 64 × 1.5 = 9,600 HP
Mass: 64 × 0.9 = 57.6 kg
Cost: 64 × 10 × 1.0 = 640 Titanium
```

### 🚀 Engine Blocks
```
┌────────────┬─────────────────────────────────────────┐
│  Property  │  Scaling                                │
├────────────┼─────────────────────────────────────────┤
│  Thrust    │  50 N/m³ × Volume × Material Efficiency │
│  Power Use │  5 W/m³ × Volume                        │
│  HP        │  100 × Volume × Material Durability     │
│  Mass      │  Volume × Material Mass                 │
│  Cost      │  Volume × 10 × 2.0× (specialized)       │
└────────────┴─────────────────────────────────────────┘
```

**Example**: 3×3×6 Trinium Engine
```
Volume: 54 m³
Thrust: 50 × 54 × 1.5 = 4,050 N
Power: 5 × 54 = 270 W
HP: 100 × 54 × 2.5 = 13,500 HP
Mass: 54 × 0.6 = 32.4 kg
Cost: 54 × 10 × 2.0 = 1,080 Trinium
```

### ⚡ Generator Blocks
```
┌─────────────┬────────────────────────────────────────┐
│  Property   │  Scaling                               │
├─────────────┼────────────────────────────────────────┤
│  Power Gen  │  100 W/m³ × Volume × Material Eff.     │
│  Storage    │  50 W per generator                    │
│  HP         │  100 × Volume × Material Durability    │
│  Mass       │  Volume × Material Mass                │
│  Cost       │  Volume × 10 × 2.5× (highly valued)    │
└─────────────┴────────────────────────────────────────┘
```

### 🛡️ Shield Generator Blocks
```
┌─────────────┬────────────────────────────────────────┐
│  Property   │  Scaling                               │
├─────────────┼────────────────────────────────────────┤
│  Capacity   │  200 × Volume × Material Shield Mult.  │
│  Power Use  │  10 W/m³ × Volume                      │
│  HP         │  100 × Volume × Material Durability    │
│  Mass       │  Volume × Material Mass                │
│  Cost       │  Volume × 10 × 3.0× (premium)          │
└─────────────┴────────────────────────────────────────┘
```

### 📦 Cargo Blocks
```
┌─────────────┬────────────────────────────────────────┐
│  Property   │  Scaling                               │
├─────────────┼────────────────────────────────────────┤
│  Capacity   │  100 units/m³ × Volume                 │
│  HP         │  100 × Volume × Material Durability    │
│  Mass       │  Volume × Material Mass                │
│  Cost       │  Volume × 10 × 1.2× (cheap storage)    │
└─────────────┴────────────────────────────────────────┘
```

---

## 💰 Cost Calculation System

### Base Cost Formula
```
Cost = Volume × BaseCost × TypeMultiplier × MaterialMultiplier

Where:
  BaseCost = 10 units per m³
  TypeMultiplier = Based on block specialization
  MaterialMultiplier = Material tier durability
```

### Type Multipliers
```
┌──────────────────┬─────────┬──────────────────────┐
│  Block Type      │  Multi  │  Reasoning           │
├──────────────────┼─────────┼──────────────────────┤
│  Hull            │  1.0×   │  Basic structure     │
│  Armor           │  1.5×   │  Reinforced          │
│  Cargo           │  1.2×   │  Simple storage      │
│  Crew Quarters   │  1.3×   │  Life support        │
│  PodDocking      │  1.5×   │  Specialized dock    │
│  Gyro Array      │  1.5×   │  Precision systems   │
│  Thruster        │  1.8×   │  Maneuvering         │
│  Engine          │  2.0×   │  Propulsion tech     │
│  Turret Mount    │  2.0×   │  Weapon platform     │
│  Generator       │  2.5×   │  Power technology    │
│  Shield Gen      │  3.0×   │  Advanced shielding  │
│  Hyperdrive Core │  3.5×   │  FTL technology      │
└──────────────────┴─────────┴──────────────────────┘
```

### Material Multipliers
```
Iron:     1.0× (Durability Mult.)
Titanium: 1.5×
Naonite:  2.0×
Trinium:  2.5×
Xanion:   3.0×
Ogonite:  4.0×
Avorion:  5.0×
```

### Example Cost Calculation
```
Block: 4×4×4 Xanion Shield Generator

Step 1: Calculate Volume
  Volume = 4 × 4 × 4 = 64 m³

Step 2: Apply Multipliers
  Cost = 64 × 10 × 3.0 × 3.0
  Cost = 64 × 90
  Cost = 5,760 Xanion

Result: Expensive but powerful!
```

---

## 🎯 Strategic Building Tips

### Efficiency vs. Power
```
Small Blocks (2×2×2):
  ✓ Precise placement
  ✓ Lower individual cost
  ✓ More flexible layouts
  ✗ Lower total stats
  ✗ More blocks needed

Large Blocks (8×8×8):
  ✓ Massive stats per block
  ✓ Fewer total blocks
  ✓ Simpler management
  ✗ Less precise
  ✗ Higher individual cost
```

### Optimal Sizing Strategy
```
Critical Systems (Generators, Shields):
  └─▶ Make LARGE for maximum output
      └─▶ 6×6×6 or bigger recommended

Engines:
  └─▶ Balance size with power consumption
      └─▶ 4×4×4 is sweet spot

Armor:
  └─▶ Stretch thin plates for coverage
      └─▶ 8×1×8 provides great HP/volume ratio

Cargo:
  └─▶ Make MASSIVE for efficiency
      └─▶ 10×10×10 for freight ships
```

---

## 🏗️ Ship Design Examples

### ⚔️ Fighter Configuration
```
Compact & Agile Design:
┌──────────────────────────────────────────────┐
│  Core Hull:        3×3×6  (compact)          │
│  Engines (×2):     2×2×3  (good thrust)      │
│  Generator:        3×3×3  (adequate power)   │
│  Shield Gen:       2×2×2  (minimal)          │
│  Weapons (×4):     -      (turret mounts)    │
│                                              │
│  Result: Fast, maneuverable, fragile         │
└──────────────────────────────────────────────┘
```

### 🚚 Freighter Configuration
```
Cargo-Optimized Design:
┌──────────────────────────────────────────────┐
│  Cargo Bay:        12×12×20 (MASSIVE)        │
│  Hull Frame:       1×1×1 plates (minimal)    │
│  Engines (×4):     3×3×4 (to move cargo)     │
│  Generator (×2):   4×4×4 (power hungry)      │
│  Shield Gen:       4×4×4 (valuable cargo)    │
│                                              │
│  Result: 28,800 m³ cargo capacity!           │
└──────────────────────────────────────────────┘
```

### 🛡️ Tank Configuration
```
Heavily Armored Design:
┌──────────────────────────────────────────────┐
│  Armor Plates:     10×2×10 (layered)         │
│  Hull Core:        6×6×8  (strong center)    │
│  Generators (×3):  5×5×5  (power hungry)     │
│  Shield Gens (×2): 4×4×4  (double shields)   │
│  Engines (×6):     3×3×3  (compensate mass)  │
│                                              │
│  Result: 500,000+ HP, unstoppable            │
└──────────────────────────────────────────────┘
```

---

## 📊 Build Preview System

### Before Placing a Block
The system shows you:
```
┌─────────────────────────────────────────────┐
│  BLOCK PREVIEW STATS                        │
├─────────────────────────────────────────────┤
│  Size:          6 × 4 × 6 meters           │
│  Volume:        144 m³                      │
│  Material:      Trinium                     │
│  Type:          Generator                   │
│                                             │
│  ─────── Stats ───────                      │
│  Mass:          86.4 kg                     │
│  HP:            36,000                      │
│  Power Gen:     21,600 W                    │
│  Power Storage: 50 W                        │
│                                             │
│  ─────── Cost ────────                      │
│  Materials:     3,600 Trinium               │
│                                             │
│  [Place Block]  [Cancel]                    │
└─────────────────────────────────────────────┘
```

---

## 🔧 Advanced Techniques

### 1. Layered Armor
```
Create spaced armor for maximum protection:

  Outer Layer: 8×1×8 Armor (cheap, absorbs damage)
     Gap: 2 meters
  Inner Layer: 6×2×6 Armor (thick, main protection)
     Gap: 1 meter
  Core: Critical systems

Result: Damage distributed across multiple layers
```

### 2. Modular Design
```
Build in sections that can be easily replaced:
  - Engine module (detachable)
  - Weapon pods (swappable)
  - Cargo containers (scalable)
  - Bridge section (constant)
```

### 3. Efficient Power Routing
```
Place generators centrally:
  ┌────────────┐
  │  Weapons   │
  │            │
  ├─Generator──┤ ← Center
  │            │
  │  Shields   │
  └────────────┘

Result: Equal power distribution, less waste
```

---

## ⚠️ Common Mistakes

### ❌ All Small Blocks
```
Problem: 1000× (2×2×2) blocks
Issues:
  - Tedious to place
  - Lower total efficiency
  - Performance impact

Solution: Use larger blocks for main structure
```

### ❌ Unbalanced Proportions
```
Problem: 20×20×1 pancake ship
Issues:
  - Easy to hit from top/bottom
  - Poor structural integrity
  - Unrealistic

Solution: Aim for 2:1:1 or 3:2:1 ratio
```

### ❌ Ignoring Material Costs
```
Problem: Everything in Avorion
Issues:
  - Prohibitively expensive
  - Wasted on non-critical blocks

Solution: Mix materials strategically
  - Avorion: Shields, Generators
  - Trinium: Engines, Weapons
  - Iron/Titanium: Hull, Cargo
```

---

## 🎓 Master Builder Checklist

Before finalizing your design:
```
□ Power Budget Calculated
  └─ Generation > Consumption ✓

□ Mass Distribution Balanced
  └─ Center of Mass centered ✓

□ Sufficient Thrust
  └─ Thrust/Mass Ratio > 1.0 ✓

□ Adequate Defense
  └─ HP Pool > Expected Damage ✓

□ Cost Within Budget
  └─ Materials available ✓

□ Aesthetic Appeal
  └─ Looks awesome ✓
```

---

```
╔═══════════════════════════════════════════════════════════════════════════╗
║                  BUILD SMART, FLY STRONG, CONQUER ALL                     ║
║              Every block matters. Every choice counts.                    ║
╚═══════════════════════════════════════════════════════════════════════════╝
```

**Last Updated**: November 2025  
**Version**: 1.0  
**Status**: 🔨 UNDER CONSTRUCTION (But Functional!)
