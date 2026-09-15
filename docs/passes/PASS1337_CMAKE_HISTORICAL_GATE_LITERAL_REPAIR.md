# Pass1337 — CMake Historical-Gate Literal Repair

The Pass1336 logic was semantically correct but its self-certification used this
kind of token inside a quoted CMake `foreach` item:

```cmake
"file(READ \"${ROOT}/src/application/NativeBattlefieldRenderer.cpp\" RENDERER)"
```

CMake expands `${ROOT}` before `string(FIND)` runs.

The target file being inspected correctly contains the literal source text:

```cmake
file(READ "${ROOT}/src/application/NativeBattlefieldRenderer.cpp" RENDERER)
```

Therefore Pass1336 searched for an expanded absolute Windows path inside a source
file that contains `${ROOT}`, causing the only remaining CTest failure.

Pass1337 replaces that fragile self-search with stable semantic substrings and adds
a guard preventing the expanded-ROOT form from returning.

No runtime/editor/game source is changed.
