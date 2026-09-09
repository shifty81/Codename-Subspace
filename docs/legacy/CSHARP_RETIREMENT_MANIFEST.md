# C# Retirement Manifest — Conversion Complete

Date: 2026-08-30

The production Codename: Subspace runtime is now native C++. The legacy `AvorionLike/**/*.cs` tree is retained only as an inert historical/reference archive and is excluded from the active solution, CMake graph, setup scripts, Full Gate, and shipping runtime.

## Exhaustive classification

- Total legacy `.cs` files: **239**
- Native replaced: **182**
- Superseded by current Subspace design: **10**
- Reference-only algorithms: **17**
- Historical reference only: **30**
- Deferred/unclassified: **0**

The exhaustive file-by-file mapping is in `docs/legacy/CSHARP_RETIREMENT_MANIFEST.csv`.

## Production authority

The active Visual Studio solution contains only `SubspaceEngine`, `SubspaceGame`, and `SubspaceTests`. `AvorionLike/AvorionLike.csproj` is inert, has no SDK or `PackageReference`, and cannot produce the shipping game. The native regression guard fails the Full Gate if a C# project is reintroduced into the solution, managed packages return, managed commands return to operational build scripts, or managed files enter `engine/`.

## What “conversion complete” means

It does **not** mean every old prototype experiment was copied line-for-line. That would reintroduce obsolete design decisions and duplicate authorities. It means every legacy runtime responsibility has one of these terminal dispositions: native replacement, deliberate supersession, or reference-only retirement. No C# item is required to build, launch, simulate, render, save, or operate the current game.
