# Codename: Subspace Dependencies

## Production runtime

Codename: Subspace is native C++17.

Required build/runtime dependencies are intentionally small:

- C++17 compiler
- CMake 3.16+
- OpenGL on the native game/platform target
- Windows SDK + Visual Studio 2022 Desktop development with C++ for the primary Windows build

Optional development tooling:

- Git
- Python 3 for project/content validation utilities
- Ninja for faster local CMake builds

## Explicitly retired managed dependencies

The production build no longer depends on:

- .NET / CLR
- NuGet
- Silk.NET
- NLua
- AssimpNet
- SixLabors.ImageSharp

`AvorionLike/` is a historical/reference archive only. Its inert `.csproj` has no SDK or package references.

## Asset strategy

Game-neutral assets and data are reused directly. OBJ loading is native through `core/resources/ObjAssetLoader`; richer asset formats should be normalized by native/offline import tooling rather than by restoring AssimpNet to the shipping game.

## Verification

On Windows run:

```powershell
.\SubspaceTools.ps1 -Action full-gate -Clean
```

On other platforms:

```bash
cmake -S engine -B engine/build -DSUBSPACE_BUILD_TESTS=ON
cmake --build engine/build
ctest --test-dir engine/build --output-on-failure
```
