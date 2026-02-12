# Visual Reference Guide

This document describes the target visual style for Codename-Subspace, based on the
reference image (`ChatGPT Image Feb 10, 2026, 06_37_38 PM.png`) and the Avorion ship
building document (`avorion ship.rtf`).

## Reference Image Overview

The reference image shows a heavy, industrial capital ship in a deep-space environment
with asteroids and a nebula skybox. Every visual element in the game should aim to
match or closely emulate this look.

---

## Ship Visual Style

### Hull Appearance
- **Color**: Dark gunmetal grey / charcoal (`~0.25, 0.24, 0.23` RGB normalized)
- **Finish**: Weathered metallic with visible panel lines and surface detail
- **Roughness**: Medium-high (0.45–0.55) — not mirror-shiny, but clearly metallic
- **Metallic**: High (0.85–0.92) — reads as metal, not plastic

### Block Construction
- Ships are assembled from **rectangular/cube blocks** in a grid pattern
- Visible seams and panel separations between blocks
- Stepped/layered construction — blocks are stacked in tiers
- Strong **bilateral (X-axis) symmetry**
- Wedge and tapered sections at bow and stern

### Emissive / Glow Elements
- **Engine exhausts**: Bright orange-amber glow (`~1.0, 0.6, 0.15`) with trailing light
- **Weapon ports**: Orange-amber points of light
- **Indicator lights**: Small red-orange dots on hull panels (`~1.0, 0.3, 0.1`)
- Emissive elements provide the primary color contrast against the dark hull

### Lighting on Hull
- Primary light from above-right (warm white)
- Cool blue-teal fill light from below (reflecting nebula environment)
- Strong ambient occlusion in block crevices and between panel lines

---

## Asteroid Visual Style

### Shape
- Irregular, rocky, non-spherical shapes
- Jagged edges and rough surfaces
- Scattered in loose clusters

### Color / Material
- **Base color**: Very dark grey-brown (`~0.18, 0.16, 0.14`)
- **Surface variation**: Slightly lighter patches of brown/tan
- **Roughness**: Very high (0.85–0.95) — rough, non-reflective rock
- **Metallic**: Very low (0.05–0.15) — natural rock, not metal

---

## Skybox / Space Background

### Nebula
- Dominant **teal/cyan-green** nebula clouds (`~0.05, 0.15, 0.12`)
- Secondary **warm amber/golden** nebula highlights (`~0.15, 0.10, 0.03`)
- Nebula is soft and atmospheric, not bright or overpowering
- Creates a moody, deep-space atmosphere

### Stars
- Subtle star field, not overly dense
- Mostly dim white/blue-white points
- A few brighter stars scattered throughout

### Overall Mood
- Dark, atmospheric, industrial
- High contrast between the dark ship/asteroids and the glowing engine elements
- Cool teal-green environment with warm orange accent lighting

---

## Mapping to Avorion Ship Document

The `avorion ship.rtf` document provides the system architecture for building ships
that look like the reference image. Key principles:

1. **Data-first construction**: Ships are block arrays, not meshes
2. **Limited shape set**: Cube, Rect, Wedge, Corner, Slope only
3. **Grid snapping**: Integer coordinates, 90° rotation increments
4. **Symmetry enforcement**: X/Y/Z mirror planes
5. **Material-driven visuals**: Color and finish come from material type, not unique textures
6. **Instanced rendering**: Batch by shape + material for performance
7. **Functional blocks**: Every block affects ship stats (mass, thrust, power, HP)
8. **Per-block damage**: Destruction removes individual blocks

### Material Tier Visual Progression
Per the document, materials should progress in visual quality:
- **Iron**: Dark gunmetal, weathered, industrial (most common — matches reference hull)
- **Titanium**: Slightly lighter steel-grey
- **Naonite**: Dark base with subtle green emissive accents
- **Trinium**: Dark base with subtle blue emissive accents
- **Xanion**: Dark base with golden emissive accents
- **Ogonite**: Dark base with orange-red emissive accents (matches reference engine glow)
- **Avorion**: Dark base with purple emissive accents
