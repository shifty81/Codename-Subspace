# How to Build and Run Codename: Subspace

Codename: Subspace now has one production runtime: **native C++**. The old C# prototype is retained only as reference material.

## Windows — recommended

Requirements:

- Visual Studio 2022
- Desktop development with C++ workload
- Windows SDK
- CMake

From the repository root:

```powershell
.\SubspaceTools.ps1
```

Use **1. FULL GATE** to apply root-drop patches, audit the project, build `SubspaceEngine`, `SubspaceGame`, and `SubspaceTests`, run tests, perform the native runtime smoke, and create a debug bundle.

Use **2. Run native C++ game** to launch the current game runtime.

The historical solution filename `AvorionLike.sln` is retained for compatibility, but the solution itself is C++ only.

## Command-line CMake

```bash
cmake -S engine -B engine/build -DSUBSPACE_BUILD_TESTS=ON
cmake --build engine/build
ctest --test-dir engine/build --output-on-failure
```

The game executable is:

- Windows: `subspace_game.exe`
- Linux/macOS test builds: `subspace_game`

## Native-runtime guard

The Windows Full Gate runs `scripts/subspace_native_runtime_guard.ps1`. It rejects reintroduction of an active C# project, managed package references, managed operational build commands, or managed source under `engine/`.

## Legacy archive

Do not use `dotnet build` or `dotnet run`. `AvorionLike/AvorionLike.csproj` is intentionally inert. See `docs/legacy/CSHARP_RETIREMENT_MANIFEST.md` for the complete retirement policy.
