# MANIFEST — Pass506R7 Cumulative Universal Build Controls

Supersedes Pass506R3, R4, R4A, R5 and R6.

R7 adds:
- Q/W/E/R Select/Move/Rotate/Scale tool authority
- contextual transform UI instead of simultaneous debug matrices
- CAMERA / SHIP / LOCAL transform spaces
- CAMERA as player-friendly default
- camera-relative arrow-key movement
- Shift = 0.1x precision for movement, rotation, scale and Shipyard camera
- Home = Frame Ship
- selected-module uniform scaling
- whole-assembly scaling around assembly center
- scale persistence in VisualModulePlacement
- 0.10x–4.00x scale safety clamp
- semantic-successor updates to R3/R4/R5 GUI tests
- Pass506R7 regression suite

Retains:
- freeform Save Draft + generator-refinement export
- R5 paint workflow
- responsive non-overlap GUI
- Wing147 orientation fixes
- propulsion material treatment
- Windows compile repair

Validation:
- native build PASS
- 43 / 43 CTest PASS
- R7 22 / 22 focused assertions PASS
