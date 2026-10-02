R158-R177 PLANETARY COMMAND

1) Apply the .patch through the normal Subspace PCC intake.
2) From the repository root run:
   powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\subspace_planetary_command_r158_r177_apply.ps1 -Root .
3) This only edits source transactionally. It does NOT configure or build C++.
4) Optional source-only check:
   python tools\control\tests\test_r158_r177_planetary_command_source.py .
