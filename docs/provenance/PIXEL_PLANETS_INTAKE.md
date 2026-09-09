# PixelPlanets Intake

Source: uploaded `PixelPlanetsSource.zip`.

Upstream project name in the uploaded archive: `PixelPlanets`.

License found in uploaded source: MIT License, copyright 2020 Deep-Fold.

Imported into Subspace as reference-only source material under:

```text
reference/third_party/pixel_planets/
```

This intake intentionally excludes:

```text
.git/
.import/
*.import
*.stex
*.md5
font files
Godot export/cache artifacts
```

The immediate implementation direction is not to embed Godot or depend on Godot runtime files. Instead, Subspace ports the useful procedural concepts into C++ systems that produce renderer-neutral `RuntimeVisualProfile` data.

## Relevant source categories

```text
Planets/Asteroids
Planets/BlackHole
Planets/DryTerran
Planets/Galaxy
Planets/GasPlanet
Planets/GasPlanetLayers
Planets/IceWorld
Planets/LandMasses
Planets/LavaWorld
Planets/NoAtmosphere
Planets/Rivers
Planets/Star
```

## Subspace destination

```text
engine/include/celestial/
engine/src/celestial/
engine/include/rendering/RuntimeVisualProfile.h
engine/src/rendering/RuntimeVisualProfile.cpp
```
