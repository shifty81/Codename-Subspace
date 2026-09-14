# Pass1268-1292 R3 Test Handoff

Apply on top of Pass1268-1292 + R1 + R2, then rerun PCC option 1.

Expected:
- build remains PASS;
- the 95 native/runtime/Shipyard CTest targets remain PASS;
- `SubspaceProjectOpsStaticCertification` no longer errors compiling the
  literal `compatibilityIndex++` source token;
- total CTest becomes 96 / 96;
- continue to main-menu Shipyard visual acceptance after GREEN.
