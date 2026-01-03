# Gameplay Vision Discussion

**Created:** January 3, 2026  
**Status:** 🔴 NEEDS CLARIFICATION - Core gameplay mechanics unclear

---

## Purpose

Before implementing additional features like quests, we need to nail down **exactly** what the core gameplay loop should be and ensure it matches your vision.

---

## Current Implementation Analysis

### What Currently Exists

#### Ship Movement
**Current Implementation:**
- **Physics-based Newtonian movement** with inertia
- **6DOF (6 Degrees of Freedom)** controls:
  - WASD - Forward/Back/Strafe Left/Right
  - Space/Shift - Up/Down
  - Arrow Keys + Q/E - Pitch/Yaw/Roll rotation
- **Physics properties** calculated from ship blocks:
  - Mass from block materials
  - Thrust from engine blocks
  - Torque from thruster blocks
  - Center of mass affects rotation
- **Emergency brake** (X key) to stop movement

**Questions:**
1. ❓ Is Newtonian physics what you want, or should it feel more arcade-like?
2. ❓ Should there be a speed limit or is unlimited velocity OK?
3. ❓ Should rotation be instant or gradual based on thrusters?
4. ❓ Do you want "flight assist" mode (auto-dampening) or manual control only?
5. ❓ Should mouse control the ship directly (like a flight sim) or just camera?

---

#### Ship Building
**Current Implementation:**
- **Build mode** accessed via 'B' key or menus
- **Voxel placement system** with grid-based positioning
- **Block types:**
  - Hull (structural)
  - Armor (protective)
  - Engine (forward thrust)
  - Thruster (directional thrust)
  - Generator (power)
  - Shield Generator (defense)
  - Cargo Bay (storage)
  - Weapon Mount (turrets)
  - Crew Quarters (crew capacity)
- **Material tiers:** Iron → Titanium → Naonite → Trinium → Xanion → Ogonite → Avorion
- **Dynamic scaling:** Blocks can be stretched in X/Y/Z axes
- **Real-time stats:** Mass, thrust, power, shields update as you build
- **Build costs:** Resources deducted from inventory

**Questions:**
1. ❓ Is the voxel grid system what you envision? (Like Minecraft/Avorion?)
2. ❓ Or do you want a different building approach? (Pre-made modules? Ship editor?)
3. ❓ Should building be done in-flight or only at stations?
4. ❓ Is the current block type variety sufficient?
5. ❓ How should block connectivity work? (Any adjacent blocks connect?)
6. ❓ Should there be integrity/structural limits?
7. ❓ Do you want symmetry tools, templates, or blueprint sharing?

---

#### Player Interaction Model
**Current Implementation:**
- **Direct ship control mode** - Player controls a ship entity
- **Camera modes:**
  - Camera Mode: Free-look camera (WASD moves camera)
  - Ship Control Mode: Camera follows ship (WASD moves ship)
  - Toggle with 'C' key
- **UI overlays** accessible via hotkeys:
  - ESC: Pause menu
  - TAB: Player status
  - I: Inventory
  - B: Ship builder
  - M: Galaxy map
  - F1/F2/F3: Various HUD panels
- **Mouse:** Currently used for camera look, cursor appears when UI is open

**Questions:**
1. ❓ Should the player BE the ship, or pilot a character inside?
2. ❓ Should there be first-person view from cockpit?
3. ❓ How should the player interact with stations? (Dock and get out? Remote menu?)
4. ❓ Should mouse be used for aiming weapons or just UI?
5. ❓ Is the current "camera mode toggle" system intuitive?

---

#### Combat System
**Current Implementation:**
- **Turret-based weapons** mounted on ship blocks
- **6 weapon types:**
  - Chaingun (rapid-fire ballistic)
  - Laser (energy beam)
  - Plasma (projectile)
  - Railgun (piercing)
  - Cannon (explosive)
  - Missile Launcher (homing)
- **Auto-targeting system** - Turrets track nearby enemies
- **Manual fire control** - Player can select targets
- **Shield system** - Absorbs damage before hull
- **Damage system** - Blocks can be destroyed individually
- **Energy management** - Weapons consume power

**Questions:**
1. ❓ Should weapons fire automatically or require player input?
2. ❓ Do you want manual aiming (crosshair) or just target selection?
3. ❓ Should combat be twitch-based or more strategic/tactical?
4. ❓ Is the turret system what you want, or fixed weapons?
5. ❓ How should shields work? (Regenerating? Directional?)

---

#### Galaxy/World Structure
**Current Implementation:**
- **Procedural galaxy:** 1000×1000×1000 sector grid
- **Sector-based travel:** Hyperdrive jumps between sectors
- **Content per sector:**
  - Asteroids (mining resources)
  - Stations (trading, services)
  - NPC ships (enemies, traders, pirates)
  - Empty space
- **Distance progression:** Better materials/difficulty closer to galaxy center
- **Tech levels** based on distance from center

**Questions:**
1. ❓ Is the sector-based galaxy structure what you want?
2. ❓ Should there be seamless space or discrete sectors?
3. ❓ How should sectors be sized? (Current: abstract grid)
4. ❓ Should there be planets? Solar systems? Or just asteroid fields?
5. ❓ Do you want more variety in sector types? (Nebulas, wormholes, etc?)

---

## What Seems Unclear

Based on your feedback, these fundamental aspects need clarification:

### 1. Core Gameplay Loop
**What should the minute-to-minute gameplay feel like?**
- [ ] Is this a builder-focused game where you spend most time designing ships?
- [ ] Is this a combat game where you fly and fight?
- [ ] Is this an exploration/trading game?
- [ ] Is this an RTS where you command fleets?
- [ ] Some combination? What's the primary loop?

### 2. Ship Control Philosophy
**How should flying feel?**
- [ ] Realistic space physics (Newtonian)?
- [ ] Arcade-style (like Wing Commander, Star Fox)?
- [ ] Submarine-style (like FTL)?
- [ ] Tank controls?
- [ ] Something else?

### 3. Building Philosophy
**How should ship construction work?**
- [ ] Voxel grid (current system - like Avorion, Space Engineers)?
- [ ] Module-based (snap together pre-made parts - like Stellaris)?
- [ ] Ship editor (paint-style - like Cosmoteer)?
- [ ] Pre-designed ships only?
- [ ] Something else?

### 4. Player Perspective
**What is the player in this universe?**
- [ ] A ship AI/consciousness?
- [ ] A pilot inside the ship?
- [ ] A commander managing fleets?
- [ ] A character that can walk around?
- [ ] Something else?

### 5. Core Mechanics Priority
**Which gameplay should be most important?**
Rank these from 1 (most important) to 6 (least important):
- [ ] Ship building/customization
- [ ] Flying/ship control
- [ ] Combat
- [ ] Trading/economy
- [ ] Exploration/discovery
- [ ] Fleet management/RTS

---

## Reference Games

To help clarify your vision, which games have the gameplay feel you're aiming for?

### Space Games for Reference:
- **Avorion** - Voxel building, sector-based travel, sandbox
- **Space Engineers** - Detailed physics, realistic building, survival
- **FTL** - Top-down, pause-to-command, roguelike
- **Stellaris** - Grand strategy, empire management
- **Elite Dangerous** - Realistic flight model, 1:1 scale galaxy
- **Star Citizen** - First-person, realistic, massive scale
- **EVE Online** - MMO, point-and-click movement, spreadsheets
- **Homeworld** - 3D RTS, fleet command
- **X4: Foundations** - First-person, complex economy, empire building
- **Kerbal Space Program** - Realistic orbital mechanics, building

**Which of these is closest to your vision?** ___________________

**What aspects specifically?** ___________________

---

## Next Steps

To proceed effectively, we need answers to:

1. **Describe your ideal gameplay session** (5-10 minute loop):
   ```
   Example: "Player starts in their ship, flies to asteroid field,
   uses mining laser on asteroids while dodging pirates, returns to
   station to sell resources, uses credits to buy better weapons,
   flies to combat zone to test new weapons..."
   ```
   
   Your answer:
   ```
   
   
   
   ```

2. **How should ship movement FEEL?**
   ```
   Example: "Like flying a fighter jet - responsive but with momentum"
   or "Like steering a submarine - slow and deliberate"
   or "Like playing an FPS - instant response to input"
   ```
   
   Your answer:
   ```
   
   
   
   ```

3. **What should ship building be like?**
   ```
   Example: "Quick and functional - place modules, test, iterate"
   or "Detailed and creative - spend hours perfecting designs"
   or "Strategic - choose from pre-made parts to optimize"
   ```
   
   Your answer:
   ```
   
   
   
   ```

4. **What's the main goal/motivation for players?**
   ```
   Example: "Survive and thrive - build empire, conquer galaxy"
   or "Creative expression - build beautiful ships"
   or "Mastery - become the best pilot/trader/fighter"
   or "Exploration - discover all secrets"
   ```
   
   Your answer:
   ```
   
   
   
   ```

5. **What makes your game unique/different?**
   ```
   What would make someone choose YOUR game over Avorion,
   Space Engineers, or other space games?
   ```
   
   Your answer:
   ```
   
   
   
   ```

---

## Important Note

**We should NOT implement more features (like quests) until we have clarity on these fundamentals.**

Once we agree on the core loop, we can:
1. Adjust existing systems to match the vision
2. Remove systems that don't fit
3. Add only features that support the core loop
4. Design UI/controls around the intended experience

**Please fill out this document with your vision, and we'll align the codebase accordingly.**
