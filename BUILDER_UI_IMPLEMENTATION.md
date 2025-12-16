# Builder UI Implementation - Issue #4456

This document describes the implementation of the comprehensive builder interface based on reference image 4456.PNG.

## Overview

The builder UI has been enhanced to match Avorion's build mode interface with the following features:

### Components Implemented

#### 1. **Resource Panel** (Top-Left)
- **Location**: Top-left corner (10, 10)
- **Size**: 400x120 pixels
- **Features**:
  - Credits display with formatted number (e.g., "£1,234,567")
  - Sector coordinates display
  - Material inventory in two columns:
    - Left: Iron, Titanium, Naonite, Trinium
    - Right: Xanion, Ogonite, Avorion
  - Color-coded material names matching their in-game appearance
  - Red highlight for zero amounts

#### 2. **Main Builder Window** (Center)
- **Location**: Centered on screen
- **Size**: 630x550 pixels
- **Features**:
  - Material tabs with color-coded names (Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion)
  - Block selection grid (8 columns x variable rows)
  - Block types displayed as buttons with short names:
    - Hull, Armor, Engine, Thrust, Gyro, Power, Shield
    - Turret, Hyper, Cargo, Crew, Dock, CPU, Battery, Field
  - Visual selection highlight (material color for selected block)
  - Tooltips showing full block type name and description
  - Grid snap and Show Grid toggles
  - Grid size slider (1-10)
  - Block size controls (X, Y, Z dimensions)
  - Placement position controls
  - Place/Remove block buttons
  - Save/Load Blueprint buttons

#### 3. **Statistics Panel** (Right)
- **Location**: Right side (width - 310, 10)
- **Size**: 300x600 pixels
- **Features**:
  - Total Blocks count
  - Total Mass (in tons)
  - Next Block Cost (material specific)
  - Hull integrity
  - Volume (cubic meters)
  - Power generation (MW)
  - Shield capacity
  - Thrust (Newtons)
  - Torque (Newton-meters)

### Files Modified

1. **AvorionLike/Core/UI/ShipBuilderUI.cs**
   - Enhanced with comprehensive builder interface
   - Added `RenderResourcePanel()` method
   - Added `RenderMainBuilderWindow()` method
   - Added `RenderBlockGrid()` method
   - Added `RenderStatsPanel()` method
   - Added `RenderDialogs()` method
   - Updated `Render()` to orchestrate all panels
   - Material selection now uses tabbed interface
   - Block selection now uses grid with visual feedback

2. **AvorionLike/Core/UI/BuilderModeUI.cs** (NEW)
   - Alternative standalone builder UI implementation
   - Can be used instead of ShipBuilderUI
   - Includes all features from reference image
   - Ready for integration with PlayerUIManager

### Key Features

#### Material Color Coding
All materials are displayed with their characteristic colors:
- **Iron**: Silver-grey (#B8B8C0)
- **Titanium**: Silver-blue (#D0DEF2)
- **Naonite**: Emerald green (#26EB59)
- **Trinium**: Sapphire blue (#40A6FF)
- **Xanion**: Gold (#FFD126)
- **Ogonite**: Orange-red (#FF6626)
- **Avorion**: Purple (#D933FF)

#### Block Selection Grid
- 8 columns layout for easy browsing
- Visual highlighting for selected blocks
- Color-coded by material
- Tooltips show full block information
- Click to select block type and material simultaneously

#### Mode Toggle
- Place mode: Add blocks to ship
- Remove mode: Delete blocks from ship
- Visual indicators for active mode

### Usage

#### Opening Build Mode
- Press **B** key to toggle builder
- Builder automatically finds first available ship
- UI opens with Iron material tab selected by default

#### Selecting Blocks
1. Click on material tab (Iron, Titanium, etc.)
2. Click on block type in the grid
3. Adjust block size if needed
4. Position cursor and place block

#### Keyboard Shortcuts
- **B**: Toggle builder
- **1-5**: Quick tool selection (when available)
- **ESC**: Close builder

### Technical Details

#### UI Framework
- Uses ImGui for rendering
- Multiple floating windows with coordinated positioning
- Semi-transparent backgrounds (0.9-0.95 alpha)
- No window decorations for immersive feel

#### Integration Points
- Works with existing `BuildSystem.cs` for block placement
- Integrates with `VoxelStructureComponent` for ship stats
- Uses `InventoryComponent` for resource tracking
- Compatible with existing blueprint save/load system

### Comparison to Reference Image 4456.PNG

The implementation matches the reference image with:
- ✅ Top-left resource panel with credits and materials
- ✅ Center block selection panel with material tabs
- ✅ Right statistics panel
- ✅ Color-coded materials
- ✅ Grid-based block selection
- ✅ Build mode indicators
- ✅ Blueprint save/load functionality

### Future Enhancements

Potential additions to match complete Avorion functionality:
- Left toolbar with tool icons (visual icons instead of text buttons)
- Bottom toolbar with camera controls
- Block shape selection (Cube, Wedge, Corner, etc.)
- Mirror mode visualization
- Transform tools (rotate, scale, move)
- Selection mode for multi-block operations
- Copy/paste functionality
- Undo/redo visualization
- Build cost calculator
- Real-time ship simulation preview

### Testing

To test the builder UI:
1. Build the project: `dotnet build`
2. Run the game: `dotnet run`
3. Start a new game or load existing save
4. Press **B** to open the builder
5. Click through material tabs to verify all materials load
6. Select different block types to verify selection works
7. Check resource panel updates when selecting materials
8. Check stats panel shows current ship information
9. Try placing blocks to verify functionality
10. Test save/load blueprint dialogs

### Known Issues

None currently. Build succeeded with only minor warnings about unused fields in BuilderModeUI.cs (which is an alternative implementation kept for reference).

### Credits

- Reference image: 4456.PNG (Avorion-style builder interface)
- Implementation: Enhanced based on existing ShipBuilderUI.cs
- Material colors: Matching MaterialProperties.cs definitions
- Block types: Using existing BlockType enum
