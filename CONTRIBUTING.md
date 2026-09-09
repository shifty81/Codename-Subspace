# Contributing to Codename: Subspace

Codename: Subspace is a native **C++17** game/runtime. The former C# prototype under `AvorionLike/` is an inert reference archive and is not a development target.

## Prerequisites

- Visual Studio 2022 with **Desktop development with C++** on Windows, or a C++17 compiler on Linux/macOS
- CMake 3.16+
- Git
- OpenGL development/runtime support for the native game target
- Python 3 for optional content/tool validation

.NET, NuGet, Silk.NET, NLua, AssimpNet, and ImageSharp are **not** production dependencies.

## Build and test

### Windows

Use the root project utility:

```powershell
.\SubspaceTools.ps1
```

Recommended workflow: **FULL GATE**. It applies queued root-drop patches, runs project/conversion/native-runtime audits, configures/builds the native targets, runs CTest, performs the Windows runtime smoke, and packages a debug bundle.

You may also open `AvorionLike.sln`; despite its historical filename, the active solution is now C++ only and contains:

- `SubspaceEngine`
- `SubspaceGame`
- `SubspaceTests`

### CMake

```bash
cmake -S engine -B engine/build -DSUBSPACE_BUILD_TESTS=ON
cmake --build engine/build
ctest --test-dir engine/build --output-on-failure
```

## Source ownership

New production work belongs under `engine/include` and `engine/src`. Do not add gameplay/runtime work to `AvorionLike/`.

The legacy C# archive may be consulted for historical algorithms or design context. Every `.cs` file has a terminal disposition in `docs/legacy/CSHARP_RETIREMENT_MANIFEST.csv`; there are no deferred C# runtime owners.

## Coding expectations

- C++17, RAII, deterministic ownership, and explicit lifetime rules.
- Keep authoritative gameplay on the 2D simulation plane; faux-3D belongs to presentation.
- Prefer one authority per system rather than compatibility shims or duplicate implementations.
- Add or update native tests for gameplay/system behavior.
- Do not introduce CLR/C++/CLI or managed package dependencies into the shipping runtime.
- Preserve reusable content/assets as engine-neutral data rather than translating data into source code.

## Before submitting changes

Run the native Full Gate on Windows or, at minimum, CMake build + CTest on another platform. The native runtime regression guard must remain green.
