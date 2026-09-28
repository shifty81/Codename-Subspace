# R32 authored-ship exemplar review pipeline

`tools/shipyard/exemplar_review_pipeline.py` is a convenience orchestrator over the existing certified R10/R11 native intake and graph compiler. It does not introduce another generator.

The source `.subspace_ship` is read-only. The pipeline writes a review directory containing candidate, audit, observational grammar and a hash manifest. Every output is explicitly unpromoted. The grammar describes observed relationships only; substitutions and runtime installation remain disabled.

This closes the workflow gap between "I authored a ship" and "show me what the learning system extracted" while preserving the later physical/visual approval gate.
