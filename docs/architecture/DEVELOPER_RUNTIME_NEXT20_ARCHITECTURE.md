# Developer Runtime Next-20 Architecture

The developer runtime is now shaped around small, composable services rather than one editor god-object.

```text
Developer Overlay / Console / AI / Native Editor
    -> DeveloperCommandBridge
    -> DeveloperModeController
    -> DeveloperEditingLayer
    -> Runtime edit adapters
    -> Registries and pipelines
```

## New service boundaries

- `RuntimeResourceRegistry` owns live resource-handler callbacks for asset kinds.
- `ComponentReflectionRegistry` owns safe component introspection/edit callbacks.
- `RuntimeSelectionService` owns selected runtime objects.
- `RuntimeGizmoDrawList` owns renderer-neutral gizmo commands.
- `DeveloperViewportBridge` owns pick-ray handoff without knowing the renderer internals.
- `RuntimeEditDiff` describes dirty state in a UI-friendly form.
- `DeveloperDiagnosticsHub` aggregates events and validation.
- `DeveloperAiCommandProtocol` wraps future AI-generated edit commands with confirmation metadata.

## Development-play target

Development Play Mode should eventually allow:

- inspect selected entity/components;
- edit runtime component fields;
- place/remove/paint ship blocks;
- hot-reload textures, models, data, and blueprints;
- view runtime edit diffs;
- use gizmos and selection bounds;
- promote accepted runtime edits to source assets only after validation.

## Important non-goals for this bundle

This bundle does not bind renderer GPU resources, parse all game data, or replace the C# prototype. It creates the boundaries needed to do those safely after a build checkpoint.
