# Advanced Player UI System - Design Document

## Overview

This document outlines the architecture for a complete player-facing UI system with drag-and-drop inventory management, separate from the existing dev/debug UI.

---

## 🎯 Goals

### 1. Separate Dev UI from Player UI
- **Dev UI**: Debug tools, console, performance metrics (existing HUDSystem, MenuSystem)
- **Player UI**: Game interactions, inventory, ship management (new implementation)

### 2. Per-Ship Inventory System
- Each ship has its own inventory
- Player pod has its own inventory
- Transfer items between ships
- Categorized storage

### 3. Drag-and-Drop Interaction
- Intuitive item movement
- Visual feedback
- Tooltip system
- Context menus

---

## 📐 Architecture

### UI Layer Separation

```
┌─────────────────────────────────────────┐
│           Graphics Window               │
├─────────────────────────────────────────┤
│  ImGui Rendering Layer                  │
├──────────────────┬──────────────────────┤
│   Dev/Debug UI   │   Player UI          │
├──────────────────┼──────────────────────┤
│ - HUDSystem      │ - PlayerUIManager    │
│ - MenuSystem     │ - InventoryPanel     │
│ - DebugConsole   │ - ShipPanel          │
│ - FuturisticHUD  │ - TransferPanel      │
│ - PerformanceUI  │ - EquipmentPanel     │
│                  │ - ModulePanel        │
└──────────────────┴──────────────────────┘
```

### Component Architecture

```
┌──────────────────────────────────────────┐
│         InventoryComponent               │
├──────────────────────────────────────────┤
│  + MineralStorage                        │
│    - Iron, Titanium, Naonite, etc.       │
│  + AmmunitionStorage                     │
│    - Shells, Missiles, Energy Cells      │
│  + ModuleStorage                         │
│    - Weapons, Shields, Engines, Utility  │
│  + CargoCapacity                         │
│  + CurrentWeight                         │
└──────────────────────────────────────────┘
```

---

## 🎨 UI Components

### 1. PlayerUIManager (Main Controller)

**Purpose:** Manages all player-facing UI panels and interactions

**Features:**
- Panel management (open/close/toggle)
- Keyboard shortcuts
- UI state persistence
- Panel positioning

**Keyboard Shortcuts:**
```
I        - Toggle Inventory
B        - Toggle Build Mode
C        - Toggle Cargo/Transfer
M        - Toggle Map
TAB      - Quick Transfer (pod ↔ ship)
ESC      - Close All Panels
```

### 2. InventoryPanel (Core UI)

**Purpose:** Display and manage ship/pod inventory

**Layout:**
```
┌─────────────────────────────────────────┐
│  INVENTORY - [Ship Name]           [X]  │
├─────────────────────────────────────────┤
│  [Minerals] [Ammunition] [Modules]      │
├─────────────────────────────────────────┤
│  ┌─────────────────────────────────┐    │
│  │  [Icon] Iron: 1,250 / 5,000     │    │
│  │  [Icon] Titanium: 450 / 5,000   │    │
│  │  [Icon] Naonite: 125 / 5,000    │    │
│  │  ...                             │    │
│  └─────────────────────────────────┘    │
│                                          │
│  Cargo: 2,500 / 10,000 (25%)            │
│  [Progress Bar ████░░░░░░░]             │
└─────────────────────────────────────────┘
```

**Features:**
- Tab-based categories (Minerals, Ammo, Modules)
- Visual item grid
- Quantity displays
- Cargo capacity indicator
- Drag-and-drop support
- Context menus (right-click)

### 3. ItemSlot Component

**Purpose:** Reusable item slot for inventory grids

**Features:**
- Item icon/sprite
- Quantity text
- Durability bar (for modules)
- Hover tooltip
- Drag source
- Drop target
- Empty state

**States:**
```
Empty     - Gray background, "+" icon
Filled    - Item icon, quantity badge
Dragging  - Semi-transparent, follows cursor
Hovering  - Highlighted border
Selected  - Bright border
```

### 4. TransferPanel

**Purpose:** Transfer items between two inventories

**Layout:**
```
┌─────────────────────────────────────────────┐
│  TRANSFER                              [X]  │
├──────────────────────┬──────────────────────┤
│   Player Pod         │  [Ship Name]         │
├──────────────────────┼──────────────────────┤
│  [Minerals Tab]      │  [Minerals Tab]      │
│  ┌────┬────┬────┐    │  ┌────┬────┬────┐   │
│  │Iron│Tita│Nao │    │  │Iron│Tita│Nao │   │
│  │1250│450 │125 │    │  │3500│980 │0   │   │
│  └────┴────┴────┘    │  └────┴────┴────┘   │
│       ↓  ↑           │       ↓  ↑          │
│   [Transfer All]     │   [Transfer All]    │
└──────────────────────┴──────────────────────┘
```

**Features:**
- Side-by-side inventory views
- Transfer buttons (← →)
- Bulk transfer options
- Filter by category
- Search functionality

### 5. ModuleEquipPanel

**Purpose:** Equip modules to ship systems

**Layout:**
```
┌─────────────────────────────────────────┐
│  EQUIPMENT - [Ship Name]           [X]  │
├─────────────────────────────────────────┤
│  Ship Slots:                            │
│  ┌──────────────────────────────┐       │
│  │ ⚡ Weapons (2/5)              │       │
│  │  [Slot 1] [Laser Turret Mk2] │       │
│  │  [Slot 2] [Empty]             │       │
│  │                               │       │
│  │ 🛡️ Defense (1/3)              │       │
│  │  [Slot 1] [Shield Booster]    │       │
│  │                               │       │
│  │ ⚙️ Systems (3/4)              │       │
│  │  [Slot 1] [Jump Drive]        │       │
│  │  [Slot 2] [Mining Laser]      │       │
│  │  [Slot 3] [Cargo Extension]   │       │
│  └──────────────────────────────┘       │
│                                          │
│  Available Modules:                     │
│  ┌────┬────┬────┬────┬────┐            │
│  │[M1]│[M2]│[M3]│[M4]│[M5]│            │
│  └────┴────┴────┴────┴────┘            │
└─────────────────────────────────────────┘
```

**Features:**
- Equipment slots by category
- Slot capacity limits
- Module stats display
- Drag to equip/unequip
- Compare stats
- Upgrade indicator

### 6. TooltipSystem

**Purpose:** Show detailed item information on hover

**Example Tooltip:**
```
┌─────────────────────────────┐
│ 🔫 Plasma Turret Mk III     │
├─────────────────────────────┤
│ Weapon Module               │
│                             │
│ Damage: 250 per shot        │
│ Fire Rate: 2.5/sec          │
│ Range: 1200m                │
│ Energy: 45 per shot         │
│                             │
│ Requirements:               │
│  Power: 30W                 │
│  Crew: 2                    │
│                             │
│ Quality: Rare               │
│ Durability: 95%             │
└─────────────────────────────┘
```

---

## 💾 Data Structures

### Enhanced Inventory Component

```csharp
public class InventoryComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    // Storage categories
    public MineralStorage Minerals { get; set; }
    public AmmunitionStorage Ammunition { get; set; }
    public ModuleStorage Modules { get; set; }
    
    // Capacity
    public float MaxCargoCapacity { get; set; } = 10000f;
    public float CurrentCargoWeight { get; set; }
    public float CargoUsagePercent => CurrentCargoWeight / MaxCargoCapacity * 100f;
    
    // Quick access
    public bool HasSpace(float weight) => CurrentCargoWeight + weight <= MaxCargoCapacity;
    public bool CanAddItem(InventoryItem item) => HasSpace(item.Weight);
}

public class MineralStorage
{
    public Dictionary<string, int> Minerals { get; set; } = new()
    {
        {"Iron", 0},
        {"Titanium", 0},
        {"Naonite", 0},
        {"Trinium", 0},
        {"Xanion", 0},
        {"Ogonite", 0},
        {"Avorion", 0}
    };
    
    public int GetAmount(string mineralType) => Minerals.GetValueOrDefault(mineralType, 0);
    public void Add(string mineralType, int amount) => Minerals[mineralType] += amount;
    public bool Remove(string mineralType, int amount)
    {
        if (GetAmount(mineralType) >= amount)
        {
            Minerals[mineralType] -= amount;
            return true;
        }
        return false;
    }
}

public class AmmunitionStorage
{
    public Dictionary<AmmoType, int> Ammunition { get; set; } = new();
    
    public enum AmmoType
    {
        Shells,          // Chaingun/Cannon
        Missiles,        // Rocket launcher
        Torpedoes,       // Heavy missiles
        EnergyCells,     // Laser/Plasma
        Railgun Slugs    // Railgun
    }
}

public class ModuleStorage
{
    public List<ModuleItem> Modules { get; set; } = new();
    public int MaxModules { get; set; } = 100;
    
    public bool CanAddModule => Modules.Count < MaxModules;
    public void AddModule(ModuleItem module) => Modules.Add(module);
    public bool RemoveModule(ModuleItem module) => Modules.Remove(module);
}
```

### Module Item

```csharp
public class ModuleItem
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public string Name { get; set; } = "";
    public ModuleType Type { get; set; }
    public ModuleCategory Category { get; set; }
    public ModuleRarity Rarity { get; set; }
    
    // Stats
    public Dictionary<string, float> Stats { get; set; } = new();
    public float Weight { get; set; }
    public float Durability { get; set; } = 100f;
    public float MaxDurability { get; set; } = 100f;
    
    // Requirements
    public float PowerRequirement { get; set; }
    public int CrewRequirement { get; set; }
    public int TechLevel { get; set; }
    
    // Visual
    public string IconPath { get; set; } = "";
    public string Description { get; set; } = "";
}

public enum ModuleCategory
{
    Weapon,      // Turrets, launchers
    Defense,     // Shields, armor
    System,      // Engines, generators
    Utility,     // Mining, scanning
    Special      // Unique modules
}

public enum ModuleType
{
    // Weapons
    LaserTurret,
    PlasmaCannon,
    RailgunTurret,
    MissileLauncher,
    ChaingunTurret,
    
    // Defense
    ShieldBooster,
    ArmorPlating,
    PointDefense,
    
    // Systems
    EngineBooster,
    PowerGenerator,
    JumpDrive,
    CloakingDevice,
    
    // Utility
    MiningLaser,
    SalvageLaser,
    CargoExtension,
    ScannerUpgrade
}

public enum ModuleRarity
{
    Common,      // White
    Uncommon,    // Green
    Rare,        // Blue
    Epic,        // Purple
    Legendary    // Orange
}
```

---

## 🎮 Drag-and-Drop System

### Implementation Strategy

**Using ImGui Drag-Drop API:**

```csharp
// Drag Source
if (ImGui.BeginDragDropSource())
{
    // Set payload (the item being dragged)
    var itemId = item.Id.ToString();
    ImGui.SetDragDropPayload("INVENTORY_ITEM", itemId);
    
    // Visual feedback
    ImGui.Text($"{item.Name} x{item.Quantity}");
    ImGui.Image(item.IconTexture, new Vector2(32, 32));
    
    ImGui.EndDragDropSource();
}

// Drop Target
if (ImGui.BeginDragDropTarget())
{
    var payload = ImGui.AcceptDragDropPayload("INVENTORY_ITEM");
    if (payload.IsValid())
    {
        var itemId = Encoding.UTF8.GetString(payload.Data);
        OnItemDropped(Guid.Parse(itemId), targetSlot);
    }
    
    ImGui.EndDragDropTarget();
}
```

### Visual States

```csharp
// Hovering over valid drop target
ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0, 1, 0, 1)); // Green

// Hovering over invalid drop target
ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(1, 0, 0, 1)); // Red

// Dragging item
ImGui.PushStyleVar(ImGuiStyleVar.Alpha, 0.6f); // Semi-transparent
```

---

## 🔧 Implementation Timeline

### Phase 1: Data Layer (Week 1, Days 1-2)
- [x] Enhanced InventoryComponent ✅ (exists, needs enhancement)
- [ ] MineralStorage class
- [ ] AmmunitionStorage class
- [ ] ModuleStorage class
- [ ] ModuleItem class

### Phase 2: UI Framework (Week 1, Days 3-5)
- [ ] PlayerUIManager (main controller)
- [ ] Base panel class
- [ ] Keyboard shortcut system
- [ ] Panel positioning system

### Phase 3: Inventory UI (Week 2, Days 1-3)
- [ ] InventoryPanel implementation
- [ ] ItemSlot component
- [ ] Category tabs
- [ ] Cargo capacity display

### Phase 4: Drag-Drop (Week 2, Days 4-5)
- [ ] Drag-drop system
- [ ] Visual feedback
- [ ] Drop validation
- [ ] Item transfer logic

### Phase 5: Advanced Features (Week 3)
- [ ] TransferPanel (ship-to-ship)
- [ ] ModuleEquipPanel
- [ ] TooltipSystem
- [ ] Context menus
- [ ] Search/filter

### Phase 6: Polish (Week 4)
- [ ] Icons and sprites
- [ ] Sound effects
- [ ] Animations
- [ ] Tutorial tooltips

---

## 🎨 Visual Design

### Color Scheme

```
Background:  rgba(20, 20, 30, 0.95)   // Dark blue-gray
Border:      rgba(100, 150, 200, 0.8) // Light blue
Highlight:   rgba(200, 220, 255, 1.0) // Bright blue
Success:     rgba(100, 255, 100, 1.0) // Green
Warning:     rgba(255, 200, 100, 1.0) // Orange
Error:       rgba(255, 100, 100, 1.0) // Red
```

### Module Rarity Colors

```csharp
Common:    rgba(200, 200, 200, 1.0) // Gray
Uncommon:  rgba(100, 255, 100, 1.0) // Green
Rare:      rgba(100, 150, 255, 1.0) // Blue
Epic:      rgba(200, 100, 255, 1.0) // Purple
Legendary: rgba(255, 150, 50, 1.0)  // Orange
```

---

## 🚀 Quick Start Implementation

### Step 1: Update InventoryComponent (Today)
Enhance existing component with categories.

### Step 2: Create PlayerUIManager (Tomorrow)
Set up UI management framework.

### Step 3: Build InventoryPanel (Day 3)
First usable UI panel.

### Step 4: Implement Drag-Drop (Day 4-5)
Core interaction mechanic.

### Step 5: Add Transfer (Next Week)
Ship-to-ship transfers.

---

## 📊 Success Metrics

### Usability:
- Can transfer 100 items in < 30 seconds
- Drag-drop feels responsive (< 16ms latency)
- Tooltips appear instantly (< 100ms)
- No clicks required for basic operations

### Performance:
- UI render time < 2ms
- Inventory search < 10ms for 1000 items
- Panel transitions smooth (60 FPS)

### Features:
- ✅ Separate dev/player UI
- ✅ Per-ship inventories
- ✅ Drag-and-drop working
- ✅ Three storage categories
- ✅ Module equipping

---

Ready to start implementing? Let's begin with enhancing the InventoryComponent!
