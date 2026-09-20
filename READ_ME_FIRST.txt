CODENAME SUBSPACE — CUMULATIVE STUDIO MODELING CLOSURE

This supersedes the earlier un-applied S15D Bulk A package. DO NOT apply Bulk A first.

1) Extract this OUTER ZIP outside the repository.
2) Put only the enclosed Codename_Subspace_STUDIO_MODELING_CLOSURE_S15D_20260920.patch in the repository root.
3) Approve it through the project-owned PCC patch intake.
4) From repository root run:
       tools\studio\ApplyModelingClosure.cmd --check
5) Only if the exact-source check reports PASS, run:
       tools\studio\ApplyModelingClosure.cmd --apply
6) Run PCC FULL QUALITY GATE.
7) Perform the hands-on acceptance in MODELING_CLOSURE_AUDIT_AND_ACCEPTANCE.md.
8) Commit/push only after gate + hands-on tests pass.

No force/reset/stash operations are part of this handoff.
