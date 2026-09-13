# Codename Subspace Internal PCC PS5.1 Bootstrap R6R3

This is a one-time overwrite bootstrap for repositories still running the pre-R6R2 internal PCC patch engine.

Extract this ZIP directly into the CodenameSubspaceNullharbor repository root with overwrite enabled. Do not place this ZIP in the root as a patch transport.

Repairs:
- replaces executable System.IO.Path.GetRelativePath usage with the PS5.1-safe project helper;
- keeps the internal PCC open after a failed root patch instead of terminating the launcher;
- preserves option 1 = FULL QUALITY GATE / CERTIFY GREEN;
- preserves option 2 = guarded COMMIT + PUSH CURRENT GREEN;
- preserves direct SubspaceTools.cmd -Action ... automation.

After extraction:
1. Launch SubspaceTools.cmd with no .patch in root and confirm the menu opens.
2. Re-download the current cumulative .patch or move the archived failed copy from updates\failed back to root.
3. Relaunch SubspaceTools.cmd, answer Y, and confirm the patch passes manifest inspection.
4. Run option 1 before option 2.
