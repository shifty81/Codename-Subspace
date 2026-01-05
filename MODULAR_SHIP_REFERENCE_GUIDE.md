# Modular Ship Building System Reference Guide

## Overview

This document provides reference information for creating detailed modular ship components based on successful modular spaceship building systems from games and asset packs. The goal is to enhance visual appeal while maintaining performance.

## Reference Systems Analyzed

### 1. GameDev.tv Sci-Fi Starships Modular Kit (PRIMARY REFERENCE)

**Key Features:**
- 100+ modular pieces for mix-and-match ship design
- 21 pre-built spaceships in 4 categories
- 9 color scheme variants for customization
- FBX format compatible with Unity, Unreal, Godot, Blender

**Ship Categories:**
- **Dropships**: Blocky, armored carriers with cargo focus
- **Fighters**: Sleek, aerodynamic with sharp wings and weaponry
- **Personal Vehicles**: High-speed, compact designs
- **Motherships**: Large vessels with cargo bays, weapons, command modules

**Modular Component Types:**
- Hull sections (rectangular, curved, angular)
- Engines and fuel modules (visible thrust systems)
- Landing gear and structural supports
- Cockpits and control modules (glass windows, sensor arrays)
- Weapon mounts and cannons
- Satellite dishes and antennas
- Cargo bays and storage containers
- Detail parts (spotlights, glass panels, vents, fuel tanks)

**Design Philosophy:**
- Functional shapes representing different roles (cargo, combat, recon)
- Blocky/angular for defense and utility (dropships, cargo)
- Sharp and curved for speed (fighters, personal vehicles)
- Low-poly optimized for game performance
- Hand-painted textures with color masks
- Drag-and-drop prefabs for rapid assembly

**Shape Characteristics:**
- **Dropships**: Rectangular hulls, heavy armor plating, flat surfaces
- **Fighters**: Wedge-shaped bodies, sharp wings, pointed noses
- **Personal Vehicles**: Compact, streamlined, minimal profile
- **Motherships**: Layered construction, visible subsystems, modular bays

### 2. Star Sparrow Modular Spaceship (Ebal Studios)

**Key Features:**
- 8 carefully designed modules optimized for variety and easy combination
- Module types: wedges, hull segments, cockpits, engines, and functional parts
- Seamless snap-together design for both fighter-sized and larger craft
- Supports both exterior hull and internal room configuration
- Polygon counts: 1400-3500 triangles per complete ship
- Modular approach with nested prefab construction
- Multiple color masks and texture support (2K/4K)

**Module Shape Design Philosophy:**
- Wedges for aerodynamic appearance
- Hull segments with panel details
- Cockpits with recessed canopy areas
- Engines with flared nozzles and visible cooling systems
- All modules designed for seamless attachment

### 2. Space Engineers

**Key Features:**
- Grid-based building system
- Blocks in cubic, rectangular, and angled forms
- Module types: cockpits, engines/thruster pods, cargo hulls, living quarters, functional blocks
- Interior grid-based with interconnected corridors and airlocks
- Frame-and-skin construction approach (skeleton + armor/aesthetic panels)

**Common Module Shapes:**
- Cockpit blocks with viewport windows
- Cube blocks for basic hull
- Wedge blocks for slopes and angles
- Cylinder blocks for rounded sections
- Thruster pods with visible nozzles
- Turret mounts with rotation mechanisms

### 3. Avorion

**Key Features:**
- Fully modular with cubes, wedges, slabs, and rounded shapes
- Custom scale, rotation, and material for each block
- Blocks categorized by function (hull, engine, hangar, decorative)
- Pseudo-interior via overlapping blocks and transparent components
- Visual and functional definition through module arrangement

**Module Design Principles:**
- Hull blocks provide durability (thicker, armored appearance)
- Engine modules show power (larger, with visible energy systems)
- Specialized blocks have unique shapes (hangars, turrets, shields)
- Decorative modules add visual variety

## Key Design Patterns from GameDev.tv Reference

### Component Variety
- **Hull segments**: Different lengths (short 2m, medium 4m, long 6m)
- **Corner pieces**: 45° and 90° angles for ship shaping
- **End caps**: Rounded, flat, and tapered nose/tail options
- **Surface detail**: Vents, panels, ribs, hatches add visual interest
- **Glass elements**: Cockpit canopies, viewports, sensor domes

### Mechanical Details
- **Landing gear**: Visible struts, hydraulics, footpads
- **Fuel tanks**: Cylindrical pods attached to hull
- **Weapon hardpoints**: Recessed mounts, visible ammunition feeds
- **Engine nozzles**: Multiple sizes, inner glow details
- **Antennas**: Satellite dishes, radio masts, sensor arrays

### Connection System
- Modules snap together at defined connection points
- Visible mounting flanges or docking collars
- Consistent attachment sizes (small, medium, large)
- Support for both symmetrical and asymmetrical designs

## Module Shape Guidelines for Small Ships

Based on the reference systems (especially GameDev.tv), here are specific guidelines for small ship modules:

### Small Ship Size Reference
- **Small ships**: Fighter-class vessels, 10-30 meters length
- **Module size**: 1-3 meters per module
- **Polygon budget**: 50-150 triangles per small module
- **Total ship**: 1000-3000 triangles for complete small ship

### Module Categories and Shapes

#### 1. Small Cockpit Modules
**Shape Characteristics:**
- Streamlined nose cone (tapered from 2.5m to 1m width)
- Recessed canopy area for pilot visibility
- Angular panels for structural definition
- Asymmetric details (vents, sensor arrays)
- Multi-section design (mounting base → transition → canopy → nose)

**Geometric Details:**
- Front viewport angle: 30-45 degrees from horizontal
- Canopy recess depth: 0.3-0.5m
- Panel bevels: 0.1-0.2m depth
- Side detail elements: 0.2-0.4m protrusion

#### 2. Small Hull Sections
**Shape Characteristics:**
- Rectangular core with beveled edges
- Recessed panel sections (0.2-0.3m depth)
- Connection ports front/back (visible mounting rings)
- Raised structural frames/ribs
- Slight taper for flow (5-10 degree angle)

**Geometric Details:**
- Panel recess: 4-6 panels per 4m length
- Frame ribs: Every 1-2 meters
- Connection port diameter: 1.5-2m
- Edge bevel: 15-30 degree angle

#### 3. Small Engines
**Shape Characteristics:**
- Cylindrical housing (octagonal approximation for efficiency)
- Flared exhaust nozzle (10-20% wider than housing)
- Inner nozzle detail (visible exhaust port)
- Cooling fins (4-6 fins radiating outward)
- Progressive diameter stages (base → mid → nozzle)

**Geometric Details:**
- Housing octagon: 8 sides, 1-1.5m diameter
- Nozzle flare: 1.2-1.8m diameter
- Cooling fin length: 0.3-0.5m
- Fin thickness: 0.1m
- Inner port depth: 0.3-0.5m

#### 4. Small Thrusters
**Shape Characteristics:**
- Compact hexagonal housing
- Tapered nozzle design
- Thrust vectoring vanes (4 directional)
- Inner exhaust detail
- Mounting base plate

**Geometric Details:**
- Housing hexagon: 6 sides, 0.8-1.2m diameter
- Nozzle taper: 60-70 degree angle
- Vane length: 0.2-0.3m
- Base plate: Square, 1-1.5m per side

#### 5. Small Wings
**Shape Characteristics:**
- Multi-segment tapered design (root → mid → outer → tip)
- Leading edge definition (sharper, forward-facing)
- Trailing edge (gentler slope)
- Panel detail sections (raised or recessed)
- Pointed wing tip

**Geometric Details:**
- Root thickness: 0.8-1.2m
- Tip thickness: 0.2-0.4m
- Wing span: 2-4m from root to tip
- Panel segments: 3-4 per wing
- Leading edge angle: 20-30 degrees

#### 6. Small Weapon Mounts
**Shape Characteristics:**
- Layered turret structure (base → rotation ring → turret body → barrel mount)
- Visible rotation mechanism
- Mounting actuators (visible mechanical details)
- Hardpoint for barrel attachment
- Compact profile (1-2m height)

**Geometric Details:**
- Base plate: Octagonal, 1-1.5m diameter
- Rotation ring: 0.1-0.2m thick
- Turret body: 0.8-1.2m diameter
- Barrel mount: 0.3-0.5m diameter
- Actuator size: 0.2m cubes or cylinders

## Detailed Module Enhancement Strategy

### Phase 1: Small Ship Modules (Current Priority)

**Focus Areas:**
1. **Small Cockpit** - Add more canopy detail, sensor arrays, viewport panels
2. **Small Hull Section** - More panel recesses, structural frames, connection detail
3. **Small Engine** - Enhanced cooling fins, nozzle detail, housing segments
4. **Small Thruster** - Better vane definition, mounting detail, hexagon refinement
5. **Small Wings** - Multi-segment construction, leading edge detail, panel lines

**Target Improvements:**
- Increase geometric detail by 50-100% (while staying under 150 triangles each)
- Add functional-looking mechanical details
- Improve visual connectivity between modules
- Create distinct silhouettes for each module type

### Phase 2: Medium Ship Modules (Future)

**Planned Enhancements:**
- Scale up small module designs
- Add more complex subsystems
- Include interior detail hints
- Polygon budget: 150-300 triangles per module

### Phase 3: Large Ship Modules (Future)

**Planned Enhancements:**
- Capital ship scale components
- Heavy armor plating details
- Large weapon systems
- Polygon budget: 300-500 triangles per module

## Polygon Budgets by Module Type

| Module Type | Small | Medium | Large | Rationale |
|-------------|-------|--------|-------|-----------|
| Cockpit | 80-120 | 150-250 | 300-500 | Complex canopy and sensor details |
| Hull Section | 50-80 | 100-180 | 200-400 | Simpler, repeatable geometry |
| Engine | 100-150 | 200-350 | 400-600 | Detailed mechanical components |
| Thruster | 60-100 | 120-220 | 250-450 | Moderate mechanical detail |
| Wing | 60-100 | 150-280 | 300-500 | Aerodynamic surface detail |
| Weapon Mount | 70-120 | 140-260 | 280-520 | Mechanical turret details |
| Cargo Bay | 50-80 | 100-200 | 200-400 | Simple container with doors |
| Power Core | 70-110 | 140-240 | 280-480 | Reactor and cooling systems |
| Sensor Array | 80-130 | 160-280 | 320-560 | Dish and antenna details |

## Visual Design Principles

### 1. Functional Appearance
- Each module's shape should suggest its function
- Engines show thrust direction with nozzles
- Sensors have dish/antenna structures
- Weapons have clear firing directions
- Cargo bays show access points

### 2. Mechanical Detail
- Add cooling fins, vents, panels
- Show structural reinforcement
- Include mounting points and actuators
- Visible energy conduits or pipes
- Surface panel variations

### 3. Visual Hierarchy
- Larger, simpler forms for primary structure
- Smaller, detailed elements for interest
- Recessed and raised sections for depth
- Smooth surfaces contrasted with mechanical details

### 4. Modular Connectivity
- Clear attachment points
- Mounting rings or flanges at connection points
- Modules should visually flow into each other
- Avoid gaps between connected modules
- Design for 6-way attachment (±X, ±Y, ±Z)

### 5. Performance Optimization
- Keep polygon counts reasonable
- Use triangles for all faces
- Avoid duplicate vertices
- Center models at origin
- Consistent scale (1 unit = 1 meter)

## Material and Texture Guidelines (Future)

While current modules use vertex coloring, future enhancements can include:

### Texture Maps
- **Diffuse/Albedo**: Base color, panel variations
- **Normal Maps**: Surface details without geometry
- **Metallic/Specular**: Material properties
- **Emissive**: Glowing elements (engines, lights)

### Color Schemes
- **Hull**: Gray, silver, dark blue
- **Engines**: Blue/orange glow, metallic housing
- **Weapons**: Red/orange energy, dark metal
- **Utility**: Green/yellow indicators, industrial colors

## Testing and Validation

### Visual Testing
1. View modules individually for shape clarity
2. Connect multiple modules to check flow
3. Build complete small ship to verify scale
4. Test from multiple camera angles
5. Verify silhouette recognition

### Performance Testing
1. Count triangles per module
2. Test rendering of 10+ ships simultaneously
3. Verify frame rate maintains 60+ FPS
4. Check memory usage with multiple instances

### Usability Testing
1. Test module attachment in builder UI
2. Verify attachment points align correctly
3. Build various ship configurations
4. Confirm modules are selectable and placeable
5. Test save/load of modular ships

## Implementation Checklist

For each new or enhanced module:

- [ ] Research reference images for similar real-world/fictional components
- [ ] Sketch basic shape and proportions
- [ ] Create OBJ geometry with target polygon count
- [ ] Add mechanical details (fins, panels, vents, etc.)
- [ ] Verify all faces are triangles
- [ ] Center at origin (0, 0, 0)
- [ ] Set correct scale (1 unit = 1 meter)
- [ ] Test model loads in AssetManager
- [ ] Update ModuleLibrary definition
- [ ] Test in ship builder
- [ ] Take screenshots for documentation
- [ ] Verify performance impact

## References

- Star Sparrow Modular Spaceship: https://assetstore.unity.com/packages/3d/vehicles/space/star-sparrow-modular-spaceship-73167
- Space Engineers Building Guide: https://steamcommunity.com/sharedfiles/filedetails/?id=2077464786
- Avorion Ship Building System
- Modular design principles from game development best practices

## Conclusion

By following these guidelines and referencing successful modular spaceship systems, we can create detailed, functional-looking ship modules that:
- Look visually appealing and professional
- Perform efficiently in real-time rendering
- Connect seamlessly with each other
- Provide clear visual distinction between module types
- Support a wide variety of ship designs

The focus on small ship modules first allows us to establish design patterns that can scale up to medium and large ship components in future phases.

---

**Document Version:** 1.0  
**Created:** January 5, 2026  
**Status:** Active Reference Guide
