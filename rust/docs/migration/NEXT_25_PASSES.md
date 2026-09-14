# Next 25 implementation passes

1. Ember discovers `CodenameSubspace.emberproject`.
2. Ember opens Subspace as a real project.
3. Forge resolves `project.control.json`.
4. Project-owned PCC runs independently.
5. Ember adapter consumes `subspace_ember_bridge::descriptor`.
6. Render BUILD workspace.
7. Render Asset Browser panel.
8. Render Ship Viewport panel.
9. Render Outliner panel.
10. Render Inspector panel.
11. Render bottom Activity dock.
12. Add draggable splitters.
13. Add tab groups.
14. Add floating-panel support through Ember host.
15. Add layout persistence.
16. Bind ToolRegistry to central contextual rail.
17. Bind CommandRegistry / F3 search.
18. Bind SelectionState between viewport/outliner/inspector.
19. Load sample `ShipBlueprint`.
20. Render module hierarchy in Outliner.
21. Render instance vs definition properties.
22. Add first selectable viewport proxy.
23. Add Move/Rotate/Scale command transactions.
24. Add History undo/redo.
25. Export first C++ donor ship fixture and parity-test its Rust load/validation.
