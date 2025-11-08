# Codename: Subspace - Showcase Guide

## 🎮 Visual Demonstration & UI Tour

This guide shows you how to experience the enhanced visual design and responsive UI improvements in Codename:Subspace.

---

## Quick Start - Showcase Mode

The fastest way to see all improvements:

```bash
cd AvorionLike
dotnet run
# Select option 19: SHOWCASE MODE
```

This will launch an impressive visual demonstration featuring:
- **Hero-class Battlecruiser** - Multi-material flagship with full systems
- **Escort Fleet** - 2 scout ships + 2 frigates
- **Populated Universe** - Asteroids, stations, and objects
- **Enhanced UI** - All visual improvements in action

---

## 🎨 UI Improvements Overview

### Visual Enhancements

#### 1. **Gradient Backgrounds**
All HUD panels now feature beautiful gradients:
- Top-to-bottom color transitions
- Semi-transparent dark overlays
- Professional depth effect

#### 2. **Glowing Effects**
Panels and elements have subtle glow:
- Outer glow on panel borders
- Inner highlights on progress bars
- Pulsing animations on interactive elements

#### 3. **Enhanced Progress Bars**
New 3-layer rendering system:
- Background with gradient
- Filled portion with glow effect
- Color-coded based on value:
  - **Green**: >50% (Healthy)
  - **Yellow**: 25-50% (Warning)
  - **Red**: <25% (Critical)

#### 4. **Color-Coded Information**
- **Credits**: Gold (₵ symbol)
- **Iron**: Gray (▣ symbol)
- **Titanium**: Blue (▣ symbol)
- **FPS Counter**: Green/Yellow/Red based on performance
- **Ship Status**: Dynamic colors based on health

#### 5. **Animated Elements**
- Radar sweep line rotation
- Corner frame pulsing effect
- Smooth transitions

---

## 📐 Responsive Design

### Supported Resolutions

The UI automatically adapts to these resolution categories:

| Category | Resolution Range | Font Scale | UI Scale |
|----------|-----------------|------------|----------|
| **Small** | < 1280x720 | 0.85x | Compact |
| **Medium** | 1280x720 - 1920x1080 | 1.0x | Standard |
| **Large** | 1920x1080 - 2560x1440 | 1.15x | Enhanced |
| **Extra Large** | > 2560x1440 | 1.3x | Premium |

### What Scales

✅ **Automatic Scaling:**
- Panel sizes and positions
- Font sizes and text spacing
- Border thickness and glow effects
- Corner frame dimensions
- Radar size and elements
- Progress bar heights
- Icon sizes

✅ **Adaptive Layout:**
- Panels positioned as percentage of screen
- Margins calculated from screen size
- Controls text adapts to available width
- Minimum/maximum size constraints enforced

---

## 🎯 HUD Layout

### Top-Left: Ship Status Panel
**Displays:**
- Hull Integrity (green/yellow/red bar)
- Energy Level (blue bar)
- Shield Status (cyan bar)
- Percentage values

**Features:**
- Gradient background
- Glowing borders
- Color-coded status bars
- Responsive text positioning

### Top-Right: Velocity & FPS
**Displays:**
- Current speed (m/s)
- Ship mass (kg)
- FPS with color coding

**Responsive:**
- Scales from 180px to 300px width
- Font adjusts for readability
- Compact on smaller screens

### Top-Right (Below): Resources
**Displays:**
- Credits (gold icon)
- Iron (gray icon)
- Titanium (blue icon)

**Features:**
- Colorful resource icons
- Formatted numbers (commas)
- Material-specific colors

### Bottom-Left: Radar
**Displays:**
- Player position (green dot with glow)
- Nearby entities (orange dots)
- Range circles (1000m radius)
- Animated sweep line

**Features:**
- Multiple opacity grid circles
- Dynamic entity tracking
- Distance-based opacity
- Scales proportionally

### Bottom-Center: Controls Hint
**Displays:**
- Key bindings
- Control modes
- Quick reference

**Adaptive:**
- Full text on large screens
- Compact text on small screens
- Bullet point format

---

## 🚀 Testing Different Resolutions

### Method 1: Window Resize (if supported)
Some systems allow window resizing:
1. Launch showcase mode
2. Resize window
3. Observe UI adapting

### Method 2: Configure Display
Change your system resolution:

**Windows:**
```
Settings → Display → Display Resolution
```

**Linux:**
```
xrandr --output <display> --mode <resolution>
```

**macOS:**
```
System Preferences → Displays → Resolution
```

### Recommended Test Resolutions
- **720p**: 1280x720 (Small category)
- **1080p**: 1920x1080 (Medium category)  
- **1440p**: 2560x1440 (Large category)
- **4K**: 3840x2160 (Extra Large category)

---

## 📊 Performance Tips

### Optimal Settings
For best visual quality:
- **VSync**: Enabled (smooth rendering)
- **Target FPS**: 60+ for fluid animations
- **Resolution**: Native display resolution

### Lower-End Systems
If experiencing low FPS:
- Lower resolution category
- Disable VSync for higher FPS
- Close other applications

---

## 🎨 UI Color Palette

### Primary Colors
- **Cyan**: `#00B4D8` - Primary UI elements
- **Teal**: `#00D9FF` - Accents and highlights
- **Dark Blue**: `#0A0E1A` - Background base

### Status Colors
- **Health Green**: `#00FF80` - Hull >50%
- **Warning Yellow**: `#FFE600` - Hull 25-50%
- **Critical Red**: `#FF3333` - Hull <25%
- **Energy Blue**: `#4DB3FF` - Energy bars
- **Shield Cyan**: `#00F5FF` - Shield bars

### Material Colors
- **Gold**: `#FFD700` - Credits
- **Gray**: `#B3B3B3` - Iron
- **Blue**: `#80B3FF` - Titanium
- **Green**: `#00FF66` - Naonite
- **Cyan**: `#00CED1` - Trinium
- **Yellow**: `#FFD700` - Xanion
- **Orange**: `#FF4500` - Ogonite
- **Purple**: `#9370DB` - Avorion

---

## 🔧 Advanced Customization

### For Developers
Modify responsive behavior in:
```
AvorionLike/Core/UI/ResponsiveUILayout.cs
```

**Key Methods:**
- `CalculateHUDLayout()` - Panel positions
- `GetFontScale()` - Text sizing
- `GetPanelSize()` - Dynamic sizing
- `Scale()` - Apply scaling factor

### UI Renderer
Enhance visual effects in:
```
AvorionLike/Core/UI/CustomUIRenderer.cs
```

**Available Methods:**
- `DrawRectGradient()` - Gradient fills
- `DrawRectWithGlow()` - Glowing panels
- `DrawProgressBar()` - Enhanced bars
- `DrawCircleFilled()` - Dots and icons

---

## 📸 Taking Screenshots

### Best Practices
1. **Launch Showcase Mode** (Option 19)
2. **Position Camera** for good angle
3. **Wait for FPS to stabilize**
4. **Capture at native resolution**

### Screenshot Locations
- Windows: Use `Win + Print Screen`
- Linux: Use `Print Screen` or screenshot tool
- macOS: Use `Cmd + Shift + 3`

### Recommended Angles
- **Overview**: Wide angle showing full fleet
- **Detail**: Close-up of hero ship materials
- **UI Focus**: Centered on HUD elements
- **Radar**: Bottom-left corner highlight
- **Combat**: Action scene with status bars

---

## 🎮 Controls Reference

### Camera Mode (Default)
- **WASD**: Move camera
- **Space/Shift**: Up/Down
- **Mouse**: Look around
- **C**: Switch to Ship Control

### Ship Control Mode
- **WASD**: Apply thrust
- **Space/Shift**: Vertical thrust
- **Arrow Keys**: Pitch/Yaw
- **Q/E**: Roll
- **X**: Emergency brake
- **C**: Switch to Camera

### UI Controls (Always Available)
- **TAB**: Player Status
- **I**: Inventory
- **B**: Ship Builder
- **F1-F4**: Debug panels
- **ESC**: Exit to menu

---

## 🌟 HTML Demo Viewer

Generate an interactive HTML showcase:

```bash
dotnet run
# Select option 20: Generate HTML Demo Viewer
```

This creates `GAME_SHOWCASE.html` with:
- Feature highlights
- Visual statistics
- Complete controls guide
- Responsive web design
- Professional styling

Open in any web browser to share!

---

## 📚 Additional Resources

- **[README.md](README.md)** - Main documentation
- **[HOW_TO_BUILD_AND_RUN.md](HOW_TO_BUILD_AND_RUN.md)** - Build instructions
- **[UI_GUIDE.md](UI_GUIDE.md)** - Detailed UI framework guide
- **[GRAPHICS_GUIDE.md](GRAPHICS_GUIDE.md)** - Graphics system details

---

## 🐛 Troubleshooting

### UI Elements Too Small
- Increase display resolution
- Check DPI scaling settings
- Verify resolution category detection

### UI Elements Too Large
- Decrease display resolution
- Check if running on very high-res display
- Review scale factor calculations

### Text Overlapping
- Ensure proper font scaling
- Check window minimum size
- Verify aspect ratio

### Performance Issues
- Lower resolution
- Disable glow effects (modify code)
- Reduce entity count in showcase

---

## 🎉 Enjoy the Enhanced UI!

The responsive UI system ensures Codename:Subspace looks great on any display, from laptop screens to ultra-wide monitors and 4K displays!

**Next Steps:**
1. Try showcase mode
2. Test different resolutions
3. Explore all UI features
4. Generate HTML demo
5. Share screenshots!

Happy exploring! 🚀✨
